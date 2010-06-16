/*
 *  worker.cpp
 *  pod-ssh
 *
 *  Created by Anar Manafov on 16.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
//=============================================================================
#include "worker.h"
// MiscCommon
#include <SysHelper.h>
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CWorker::CWorker( const string &_id, const string &_addr,
                  const string &_sshOptions, const string &_wrkDir,
                  size_t _nNumber ):
    m_addr( _addr ),
    m_sshOptions( _sshOptions )
{
    // m_id is <_id>_<_nNumber>
    stringstream ss;
    ss << _id << "_" << _nNumber;
    m_id = ss.str();

    // constructing a full path of the worker for this id
    // pattern: <_wrkDir>/<m_id>
    m_wrkDir = _wrkDir;
    smart_append( &m_wrkDir, '/' );
    smart_path( &m_wrkDir );
    m_wrkDir += m_id;
}
//=============================================================================
void CWorker::printInfo( ostream &_stream ) const
{
    _stream << "[" << m_id << "] "
            << m_addr << ":" << m_wrkDir;
}
//=============================================================================
