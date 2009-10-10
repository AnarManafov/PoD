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
#include "Node.h"
#include "ThreadPool.h"

//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
extern sig_atomic_t graceful_quit;
//=============================================================================
namespace PROOFAgent
{

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
        if ( m_fdSignalPipe < 0 )
            CriticalErrLog( errno, "Can't open a signal pipe: " + errno2str() );
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
                task->second->dealWithData( task->first );
                //  task->second->setInUse( false );
                //  DebugLog( erOK, "done processing" );

                // report to the owner that socket is free to be added back to the "select"
                if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
                    FaultLog( erError, "Can't signal via a named pipe: " + errno2str() );

                delete task;
                task = NULL;
            }
            m_threadAvailable.notify_all();
        }
        while ( !m_stopped );

        InfoLog( erOK, "stopping a thread worker." );
    }

//=============================================================================
    void CThreadPool::pushTask( MiscCommon::INet::Socket_t &_fd, CNode* _node )
    {
        boost::mutex::scoped_lock lock( m_mutex );
        _node->setInUse( true );
        task_t *task = new task_t( _fd, _node );
        m_tasks.push( task );
        // report if queued too many tasks
//        if ( m_tasks.size() > ( m_threads.size() ) )
//        {
//            stringstream ss;
//            ss << "*** Queued " << m_tasks.size() << " tasks ***";
//            DebugLog( erOK, ss.str() );
//        }

       // DebugLog( erOK, "task is ready" );

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
