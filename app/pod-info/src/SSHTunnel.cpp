//
//  SSHTunnel.cpp
//  PoD
//
//  Created by Anar Manafov on 16.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "SSHTunnel.h"
// API
#include <signal.h>
// STD
#include <fstream>
// MiscCommon
#include "SysHelper.h"
#include "Process.h"
#include "INet.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CSSHTunnel::~CSSHTunnel()
{
    killTunnel();
}
//=============================================================================
void CSSHTunnel::setPidFile( const string &_filename )
{
    m_pidFile = _filename;
}
//=============================================================================
pid_t CSSHTunnel::pid()
{
    ifstream f( m_pidFile.c_str() );
    if( !f.is_open() )
    {
        m_pid = 0;
        return m_pid;
    }
    f >> m_pid;
    return m_pid;
}
//=============================================================================
void CSSHTunnel::killTunnel()
{
    if( 0 != m_pid )
    {
        kill( m_pid, SIGKILL );
        m_pid = 0;
    }

    unlink( m_pidFile.c_str() );
}
//=============================================================================
void CSSHTunnel::create( const CEnvironment &_env, const SOptions &_opt )
{
    // delete tunnel's file
    killTunnel();
    // create an ssh tunnel on PoD Server port
    switch( fork() )
    {
        case - 1:
            // Unable to fork
            throw runtime_error( "Unable to create an SSH tunnel." );
        case 0:
            {
                // create SSH Tunnel
                string cmd( "$POD_LOCATION/bin/private/pod-ssh-tunnel" );
                smart_path( &cmd );

                string pid_arg( "-f" );
                pid_arg += m_pidFile;

                string l_arg( "-l" );
                l_arg += _opt.m_sshConnectionStr;
                stringstream p_arg;
                p_arg << "-p" << _env.serverPort();
                string o_arg( "-o" );
                o_arg += _opt.m_openDomain;

                string sBatch;
                if( _opt.m_batchMode )
                    sBatch = "-b";

                execl( cmd.c_str(), "pod-ssh-tunnel",
                       pid_arg.c_str(),
                       l_arg.c_str(), p_arg.str().c_str(),
                       o_arg.c_str(), sBatch.c_str(), NULL );
                // we shoud never come to this point of execution
                exit( 1 );
            }
    }
    // wait for tunnel to start
    short count( 0 );
    const short max_try( 50 );
    pid();
    while( 0 == m_pid || !IsProcessExist( m_pid ) ||
           0 != MiscCommon::INet::get_free_port( _env.serverPort() ) )
    {
        ++count;
        pid();
        if( count >= max_try )
            throw runtime_error( "Can't setup an SSH tunnel." );
        usleep( 50000 ); // delays for 0.05 seconds
    }
}
//=============================================================================
