/*
 *  worker.h
 *  pod-ssh
 *
 *  Created by Anar Manafov on 16.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
#ifndef WORKER_H
#define WORKER_H
//=============================================================================
// std
#include <iosfwd>
// MiscCommon
#include "def.h"
// pod-ssh
#include "config.h"
#include "threadPool.h"
#include "local_types.h"
// boost
#include <boost/thread/mutex.hpp>
//=============================================================================
enum ETaskType {task_submit, task_clean, task_status, task_exec};
// boost::mutex is not copyable, we therefore should wrap it
typedef boost::shared_ptr<boost::mutex> mutexPtr_t;
//=============================================================================
class CWorker: public CTaskImp<CWorker, ETaskType>
{
    public:
        CWorker( configRecord_t _rec, log_func_t _log,
                 const SWNOptions &_options );
        ~CWorker();

        void printInfo( std::ostream &_stream ) const;
        void runTask( ETaskType _param );
        bool IsLastTaskSuccess() const
        {
            return m_bSuccess;
        }

    private:
        void exec_command( const std::string &_cmd,
                           const MiscCommon::StringVector_t &_params );
        void log( const std::string &_msg );

    private:
        configRecord_t m_rec;
        log_func_t m_log;
        bool m_bSuccess;
        SWNOptions m_options;
        mutexPtr_t m_mutex;
};
#endif
