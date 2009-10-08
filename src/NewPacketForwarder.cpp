/************************************************************************/
/**
 * @file NewPacketForwarder.cpp
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-09-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "ErrorCode.h"
#include "HexView.h"
// PROOFAgent
#include "NewPacketForwarder.h"

//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
extern sig_atomic_t graceful_quit;
//=============================================================================
namespace PROOFAgent
{
//=============================================================================
// CNode
//=============================================================================
    int CNode::dealWithData( MiscCommon::INet::Socket_t _fd )
    {
        if ( !isValid() )
            return -1;

        sock_type *input = socketByFD( _fd );
        sock_type *output = pairedWith( _fd );

        m_bytesToSend = read_from_socket( *input, &m_buf );

        // DISCONNECT has been detected
        if ( m_bytesToSend <= 0 || !isValid() )
            return -1;

        sendall( *output, &m_buf[0], m_bytesToSend, 0 );

        // TODO: uncomment when log level is implemented
        //  BYTEVector_t tmp_buf( m_buf.begin(), m_buf.begin() + m_bytesToSend );
        //   ReportPackage( *input, *output, tmp_buf );
//    m_idleWatch.touch();

        return 0;
    }

//=============================================================================
    void CNode::ReportPackage( MiscCommon::INet::Socket_t _socket1, MiscCommon::INet::Socket_t _socket2,
                               const BYTEVector_t &_buf )
    {
        string strSocket1;
        MiscCommon::INet::socket2string( _socket1, &strSocket1 );
        string strSocket2;
        MiscCommon::INet::socket2string( _socket2, &strSocket2 );

        stringstream ss;
        ss
        << strSocket1 << " > " << strSocket2
        << " (" << _buf.size() << " bytes): ";
        if ( !_buf.empty() )
        {
            ss
            << "\n"
            << BYTEVectorHexView_t( _buf )
            << "\n";
        }
        DebugLog( erOK, ss.str() );
    }



//=============================================================================
// CNodeContainer
//=============================================================================
    CNodeContainer::CNodeContainer()
    {
        // TODO Auto-generated constructor stub

    }

//=============================================================================
    CNodeContainer::~CNodeContainer()
    {
        // TODO Auto-generated destructor stub
    }

//=============================================================================
    void CNodeContainer::addNode( node_type _node )
    {
    	// set sockets to O_NONBLOCK mode
    	_node->setNonblock();

        m_sockBasedContainer.insert( container_type::value_type( _node->first(), _node ) );
        m_sockBasedContainer.insert( container_type::value_type( _node->second(), _node ) );
        m_nodes.insert( _node );
    }

//=============================================================================
    void CNodeContainer::removeNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            m_sockBasedContainer.erase( found );
    }

//=============================================================================
    void CNodeContainer::removeBadNodes()
    {
        // remove bad nodes
        container_type::iterator iter = m_sockBasedContainer.begin();
        container_type::iterator iter_end = m_sockBasedContainer.end();
        for ( ; iter != iter_end; )
        {
            if ( !iter->second->isActive() && !iter->second->isValid() )
            {
                m_nodes.erase( iter->second );
                m_sockBasedContainer.erase( iter++ );
            }
            else
            {
                ++iter;
            }
        }
    }

//=============================================================================
    CNodeContainer::node_type CNodeContainer::getNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::const_iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            return found->second;

        return node_type();
    }


//=============================================================================
// CThreadPool
//=============================================================================
    CThreadPool::CThreadPool( size_t _threadsCount, const string &_signalPipePath ):
            m_stopped( false ),
            m_stopping( false )
    {
        for ( size_t i = 0; i < _threadsCount; ++i )
            m_threads.create_thread( boost::bind( &CThreadPool::execute, this ) );

        // open our signal pipe for writing
        string path( _signalPipePath );
        smart_path( &path );
        m_fdSignalPipe = open( path.c_str(), O_RDWR | O_NONBLOCK );
    }

//=============================================================================
    CThreadPool::~CThreadPool()
    {
        stop();

        close( m_fdSignalPipe );
    }

//=============================================================================
    void CThreadPool::execute()
    {
        InfoLog( erOK, "starting a thread worker." );

        do
        {
            task_t* task = NULL;

            { // Find a job to perform
                boost::mutex::scoped_lock lock( m_mutex );
                if ( m_tasks.empty() && !m_stopped )
                {
                    //DebugLog( erOK, "wait for a task" );
                    m_threadNeeded.wait( lock );
                }
                if ( !m_stopped && !m_tasks.empty() )
                {
                    //DebugLog( erOK, "taking a task from the queue" );
                    task = m_tasks.front();
                    m_tasks.pop();
                }
            }
            //Execute job
            if ( task )
            {
                //DebugLog( erOK, "processing a task" );
                int res = task->second->dealWithData( task->first );
                task->second->setInUse( false );
                //DebugLog( erOK, "done processing" );

                // report to the owner that socket is free to be added to the "select"
                if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
                    FaultLog( erError, "Can't signal via a named pipe: " + errno2str() );

                switch ( res )
                {
                    case -1:
                        // disable node if there were disconnect detected
                        task->second->disable();
                        FaultLog( erError, "A disconnect were detected" );
                        break;
                    case 0: // everything was redirected without problems
                        break;
                    default:
                        break;
                }
                delete task;
                task = NULL;
            }
            m_threadAvailable.notify_all();
        }
        while ( !m_stopped );

        InfoLog( erOK, "stopping a thread worker." );
    }

//=============================================================================
    void CThreadPool::pushTask( MiscCommon::INet::Socket_t _fd, CNode* _node )
    {
        boost::mutex::scoped_lock lock( m_mutex );
        task_t *task = new task_t( _fd, _node );
        m_tasks.push( task );
        // report if queued too many tasks
        if ( m_tasks.size() > m_threads.size() )
        {
            stringstream ss;
            ss << "*** Queued " << m_tasks.size() << " tasks ***";
            InfoLog( erOK, ss.str() );
        }


        //DebugLog( erOK, "task is ready" );

        m_threadNeeded.notify_all();
    }

//=============================================================================
    void CThreadPool::stop( bool processRemainingJobs )
    {
        {
            //prevent more jobs from being added to the queue
            boost::mutex::scoped_lock lock( m_mutex );
            if ( m_stopped ) return;
            m_stopping = true;
        }
        if ( processRemainingJobs )
        {
            boost::mutex::scoped_lock lock( m_mutex );
            //wait for queue to drain.
            while ( !m_tasks.empty() && !m_stopped )
            {
                m_threadAvailable.wait( lock );
            }
        }
        //tell all threads to stop
        {
            boost::mutex::scoped_lock lock( m_mutex );
            m_stopped = true;
        }
        m_threadNeeded.notify_all();

        m_threads.join_all();
    }

}
