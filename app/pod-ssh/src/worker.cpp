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
    params.push_back( "-i" + m_rec->m_id );
    params.push_back( "-l" + m_rec->m_addr );
    params.push_back( "-w" + m_rec->m_wrkDir );
    stringstream ss;
    ss << "-n" << m_rec->m_nWorkers;
    params.push_back( ss.str() );
    params.push_back( "-o" + m_rec->m_sshOptions );

    string outPut;
    try
    {
        do_execv( cmd, params, g_cmdTimeout, &outPut );
    }
    catch( exception &e )
    {
        log( "Failed to process the task.\n" );
        return;
    }
    if( !outPut.empty() )
    {
        ostringstream ss;
        ss << "Cmnd Output: " << outPut << "\n";
        log( ss.str() );
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

    string outPut;
    try
    {
        do_execv( cmd, params, g_cmdTimeout, &outPut );
    }
    catch( exception &e )
    {
        log( "Failed to process the task.\n" );
        return;
    }
    if( !outPut.empty() )
    {
        ostringstream ss;
        ss << "Cmnd Output: " << outPut << "\n";
        log( ss.str() );
    }
}
//=============================================================================
void CWorker::log( const std::string &_msg )
{
    // All the following calls must be thread-safe.

    // print time with RFC 2822 - compliant date format
    char timestr[200];
    time_t t = time( NULL );
    struct tm tmp;
    if( localtime_r( &t, &tmp ) == NULL )
    {
        // TODO: log it.
        return;
    }

    if( strftime( timestr, sizeof( timestr ), "%a, %d %b %Y %T %z", &tmp ) == 0 )
    {
        // TODO: log it.
        return;
    }

    // write to a pipe is an atomic operation,
    // according to POSIX we just need to be shorte than PIPE_BUF
    string out( m_rec->m_id );
    out += "\t[";
    out += timestr;
    out += "]\t";
    if( _msg.size() > PIPE_BUF )
        out += "ERROR. Message is too long.\n";
    else
        out += _msg;
    write( m_fdPipe, out.c_str(), out.size() );
}
