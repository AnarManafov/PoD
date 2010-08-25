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
//=============================================================================
enum ETaskType {task_submit, task_clean};
//=============================================================================
class CWorker: public CTaskImp<CWorker, ETaskType>
{
    public:
        CWorker( configRecord_t _rec, int _fdPipe );
        ~CWorker();

        void printInfo( std::ostream &_stream ) const;
        void runTask( ETaskType _param );

    private:
        void submit();
        void clean();

    private:
        configRecord_t m_rec;
        int m_fdPipe;
};
#endif
