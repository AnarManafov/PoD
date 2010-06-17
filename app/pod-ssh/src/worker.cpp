/*
 *  worker.cpp
 *  pod-ssh
 *
 *  Created by Anar Manafov on 16.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
//=============================================================================
// pod-ssh
#include "worker.h"
// MiscCommon
#include <SysHelper.h>
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CWorker::CWorker( configRecord_t _rec ): m_rec( _rec )
{
    // constructing a full path of the worker for this id
    // pattern: <m_wrkDir>/<m_id>
    smart_append( &m_rec->m_wrkDir, '/' );
    smart_path( &m_rec->m_wrkDir );
    m_rec->m_wrkDir += m_rec->m_id;
}
//=============================================================================
CWorker::~CWorker()
{
}
//=============================================================================
void CWorker::printInfo( ostream &_stream ) const
{
    _stream << "[" << m_rec->m_id << "] with "
            << m_rec->m_nWorkers << " workers at "
            << m_rec->m_addr << ":" << m_rec->m_wrkDir;
}
//=============================================================================
