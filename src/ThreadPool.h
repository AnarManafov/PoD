/************************************************************************/
/**
 * @file NewPacketForwarder.h
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-09-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/

#ifndef NEWPACKETFORWARDER_H_
#define NEWPACKETFORWARDER_H_
// STD
#include <queue>
#include <csignal>
// BOOST
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
// MiscCommon
#include "INet.h"
#include "LogImp.h"

namespace PROOFAgent
{

class CNode;

//=============================================================================
    class CThreadPool: public MiscCommon::CLogImp<CThreadPool>
    {
            typedef std::pair<MiscCommon::INet::Socket_t, CNode*> task_t;
            typedef std::queue<task_t*> taskqueue_t;
        public:
            REGISTER_LOG_MODULE( "ThreadPool" )

            CThreadPool( size_t _threadsCount, const std::string &_signalPipePath );
            ~CThreadPool();

            void pushTask( MiscCommon::INet::Socket_t _fd, CNode* _node );
            void execute();
            void stop( bool processRemainingJobs = false );

        private:
            boost::thread_group m_threads;
            taskqueue_t m_tasks;
            boost::mutex m_mutex;
            boost::condition m_threadNeeded;
            boost::condition m_threadAvailable;
            bool m_stopped;
            bool m_stopping;
            int m_fdSignalPipe;
    };

}

#endif /* NEWPACKETFORWARDER_H_ */
