/*
 *  logEngine.cpp
 *  pod-ssh
 *
 *  Created by Anar Manafov on 31.08.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
//=============================================================================
#include "logEngine.h"
// pod-ssh
#include "version.h"
// BOOST
#include <boost/bind.hpp>
// MiscCommon
#include "SysHelper.h"
// API
#include <limits.h> // for PIPE_BUF
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CLogEngine::~CLogEngine()
{
    stop();
}
//=============================================================================
void CLogEngine::start()
{
    m_stopLogEngine = 0;
    // create a named pipe (our signal pipe)
    // it's use to collect outputs from the threads and called shell scripts...
    m_pipeName = "$POD_LOCATION/";
    smart_path( &m_pipeName );
    m_pipeName += ".ssh_plugin_pipe";
    int ret_val = mkfifo( m_pipeName.c_str(), 0666 );
    if(( -1 == ret_val ) && ( EEXIST != errno ) )
        throw runtime_error( "Can't create a named pipe: " + m_pipeName );

    // Open the pipe for reading
    m_fd = open( m_pipeName.c_str(), O_RDWR | O_NONBLOCK );
    if(( -1 == m_fd ) && ( EEXIST != errno ) )
        throw runtime_error( "Can't opem a named pipe: " + m_pipeName );

    // Start the log engine
    m_thread = boost::thread( boost::bind( &CLogEngine::thread_worker, this, m_fd, m_pipeName ) );
}
//=============================================================================
void CLogEngine::stop()
{
    m_stopLogEngine = 1;
    this->operator()( "Stopping the log engine...\n", "**" );
    m_thread.join();
    if( m_fd > 0 )
    {
        close( m_fd );
        m_fd = 0;
    }
    unlink( m_pipeName.c_str() );
}
//=============================================================================
void CLogEngine::operator()( const string &_msg, const string &_id ) const
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
    string out( _id );
    out += "\t[";
    out += timestr;
    out += "]\t";
    if( _msg.size() > PIPE_BUF )
        out += "ERROR. Message is too long.\n";
    else
        out += _msg;
    write( m_fd, out.c_str(), out.size() );
}
//=============================================================================
void CLogEngine::thread_worker( int _fd, const string & _pipename )
{
    while( _fd > 0 && !m_stopLogEngine )
    {
        fd_set readset;
        FD_ZERO( &readset );
        FD_SET( _fd, &readset );
        int retval = ::select( _fd + 1, &readset, NULL, NULL, NULL );

        if( EBADF == errno )
            break;

        if( retval < 0 )
        {
            cerr << PROJECT_NAME << ": Problem in the log engine: " << errno2str() << endl;
            break;
        }

        if( FD_ISSET( _fd, &readset ) )
        {
            const int read_size = 64;
            char buf[read_size];
            int numread( 0 );
            while( true )
            {
                numread = read( _fd, buf, read_size );
                if( numread > 0 )
                    cout << string( buf, numread );
                else
                    break;
            }
            cout.flush();
        }
    }
}
