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

// std
#include <iosfwd>
// pod-ssh
#include "config.h"

class CWorker
{
    public:
        CWorker( configRecord_t _rec );
        ~CWorker();

        void printInfo( std::ostream &_stream ) const;

    private:
        configRecord_t m_rec;
};
#endif
