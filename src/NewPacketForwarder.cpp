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
// PROOFAgent
#include "NewPacketForwarder.h"

//=============================================================================
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

        // blocking the read operation on the second if it's already processing by some of the thread
        boost::try_mutex::scoped_try_lock lock(( input == m_first ) ? m_mutexReadFirst : m_mutexReadSecond );
        if ( !lock )
            return 1;

        m_bytesToSend = read_from_socket( *input, &m_buf );

        // DISCONNECT has been detected
        if ( m_bytesToSend <= 0 || !isValid() )
            return -1;

        sendall( *output, &m_buf[0], m_bytesToSend, 0 );

        // TODO: uncomment when log level is implemented
        // ReportPackage( *_Input, *_Output, buf );
//    m_idleWatch.touch();
        return 0;
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
        m_sockBasedContainer.insert( container_type::value_type( _node->first(), _node ) );
        m_sockBasedContainer.insert( container_type::value_type( _node->second(), _node ) );
    }

//=============================================================================
    void CNodeContainer::removeNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            m_sockBasedContainer.erase( found );
    }

//=============================================================================
    CNode *CNodeContainer::getNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::const_iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            return found->second.get();

        return NULL;
    }


//=============================================================================
// CThreadPool
//=============================================================================
    CThreadPool::CThreadPool( size_t _threadsCount )
    {
        for ( size_t i = 0; i < _threadsCount; ++i )
            m_threads.create_thread( boost::bind( &CThreadPool::execute, this ) );
    }

//=============================================================================
    CThreadPool::~CThreadPool()
    {
        stop();
    }

//=============================================================================
    void CThreadPool::execute()
    {
        InfoLog( erOK, "starting a thread worker." );

        do
        {
            task_t* task = NULL;

            // Checking whether signal has arrived
            if ( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received by worker thread." );
                stop();
                break;
            }

            { // Find a job to perform
                boost::mutex::scoped_lock lock( m_mutex );
                if ( m_tasks.empty() && !m_stopped )
                {
                    m_threadNeeded.wait( lock );
                }
                if ( !m_stopped && !m_tasks.empty() )
                {
                    task = m_tasks.front();
                    m_tasks.pop();
                }
            }
            //Execute job
            if ( task )
            {
                int res = task->second->dealWithData( task->first );
                switch ( res )
                {
                    case -1:
                        break;
                    case 0:
                        break;
                        // task is locked already, pushing it back
                    case 1:
                        m_tasks.push( task );
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
        task_t *task = new task_t( _fd, _node );
        m_tasks.push( task );
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
