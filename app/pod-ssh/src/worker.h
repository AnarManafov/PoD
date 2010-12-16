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
// pod-ssh
#include "config.h"
#include "threadPool.h"
#include "local_types.h"
//=============================================================================
enum ETaskType {task_submit, task_clean, task_status};
//=============================================================================
class CWorker: public CTaskImp<CWorker, ETaskType>
{
    public:
        CWorker( configRecord_t _rec, log_func_t _log );
        ~CWorker();

        void printInfo( std::ostream &_stream ) const;
        void runTask( ETaskType _param );
        bool IsLastTaskSuccess() const
        {
            return m_bSuccess;
        }

    private:
        void submit();
        void clean();
        void status();
        void log( const std::string &_msg );

    private:
        configRecord_t m_rec;
        log_func_t m_log;
        bool m_bSuccess;
};
#endif
