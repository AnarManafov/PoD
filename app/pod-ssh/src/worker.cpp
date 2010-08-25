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
#include "SysHelper.h"
#include "Process.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
const size_t g_cmdTimeout = 35; // in sec.
//=============================================================================
CWorker::CWorker( configRecord_t _rec, int _fdPipe ):
    m_rec( _rec ),
    m_fdPipe( _fdPipe )
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
void CWorker::runTask( ETaskType _param )
{
    switch( _param )
    {
        case task_submit:
            submit();
            break;
        case task_clean:
            clean();
            break;
        default:
            return;
    }
}
//=============================================================================
void CWorker::submit()
{
    string cmd( "$POD_LOCATION/bin/pod-ssh-submit-worker" );
    smart_path( &cmd );
    StringVector_t params;
    params.push_back( m_rec->m_addr );
    params.push_back( m_rec->m_wrkDir );
    stringstream ss;
    ss << m_rec->m_nWorkers;
    params.push_back( ss.str() );
    params.push_back( m_rec->m_sshOptions );

    // TODO: since it will be executed in threads we need a sync. output
    string outPut;
    try
    {
        do_execv( cmd, params, g_cmdTimeout, &outPut );
    }
    catch( exception &e )
    {
        ostringstream ss;
        ss << m_rec->m_id << " ---> Failed to process the task." << "\n";
        write( m_fdPipe, ss.str().c_str(), ss.str().size() );
        return;
    }
    if( !outPut.empty() )
    {
        ostringstream ss;
        ss << m_rec->m_id << " --->DBG Output: " << outPut << "\n";
        write( m_fdPipe, ss.str().c_str(), ss.str().size() );

    }
}
//=============================================================================
void CWorker::clean()
{
    string cmd( "$POD_LOCATION/bin/pod-ssh-clean-worker" );
    smart_path( &cmd );
    StringVector_t params;
    params.push_back( m_rec->m_addr );
    params.push_back( m_rec->m_wrkDir );
    params.push_back( m_rec->m_sshOptions );

    // TODO: since it will be executed in threads we need a sync. output
    string outPut;
    try
    {
        do_execv( cmd, params, g_cmdTimeout, &outPut );
    }
    catch( exception &e )
    {
        ostringstream ss;
        ss << m_rec->m_id << " ---> Failed to process the task." << "\n";
        write( m_fdPipe, ss.str().c_str(), ss.str().size() );
        return;
    }
    if( !outPut.empty() )
    {
        ostringstream ss;
        ss << m_rec->m_id << " --->DBG Output: " << outPut << "\n";
        write( m_fdPipe, ss.str().c_str(), ss.str().size() );
        
    }
}
//=============================================================================
