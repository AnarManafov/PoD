/************************************************************************/
/**
 * @file PacketForwarder.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// PROOFAgent
#include "ErrorCode.h"
#include "PacketForwarder.h"

// BOOST
#include <boost/bind.hpp>

// API
#include <signal.h>

using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;
using namespace std;

const unsigned int g_BUF_SIZE = 1024;

extern sig_atomic_t graceful_quit;

void CPacketForwarder::ThreadWorker( smart_socket *_SrvSocket, smart_socket *_CltSocket )
{
    BYTEVector_t buf( g_BUF_SIZE );
    // Macking non-blocking sockets
    _SrvSocket->set_nonblock();
    _CltSocket->set_nonblock();

    fd_set readset;
    while ( true )
    {
        FD_ZERO( &readset );
        FD_SET( *_SrvSocket, &readset );
        // Setting time-out
        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if ( ::select( *_SrvSocket + 1, &readset, NULL, NULL, &timeout ) < 0 )
        {
            FaultLog( erError, "Error while calling \"select\": " + errno2str() );
            return ;
        }

        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return ;
        }

        if ( FD_ISSET( *_SrvSocket, &readset ) )
        {
            try
            {
                *_SrvSocket >> &buf;
                if ( !_SrvSocket->is_valid() )
                {
                    InfoLog( erOK, "DISCONNECT has been detected." );
                    return ;
                }

                WriteBuffer( buf, *_CltSocket );
                ReportPackage( *_SrvSocket, buf );
                buf.clear();
                buf.resize( g_BUF_SIZE );
            }
            catch ( exception & e )
            {
                FaultLog( erError, e.what() );
                return ;
            }
        }
    }
}

ERRORCODE CPacketForwarder::Start( bool _Join )
{
    // executing PF threads
    m_thrd_serversocket = Thread_PTR_t( new boost::thread(
                                            boost::bind( &CPacketForwarder::_Start, this, _Join ) ) );
    if ( _Join )  //  Join Threads (for Client) and non-join Threads (for Server mode - server shouldn't sleep while PF is working)
        m_thrd_serversocket->join();

    return erOK;
}

ERRORCODE CPacketForwarder::_Start( bool _Join )
{
    try
    {
        while ( true )
        {
            CSocketServer server;
            server.Bind( m_nPort );
            server.Listen( 1 );
            server.GetSocket().set_nonblock();

            fd_set readset;
            FD_ZERO( &readset );
            FD_SET( server.GetSocket(), &readset );
            // Setting time-out
            timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            if ( ::select( server.GetSocket() + 1, &readset, NULL, NULL, &timeout ) < 0 )
            { // TODO: Send errno to log
                FaultLog( erError, "Error while calling \"select\"" );
                return erError;
            }

            // Checking whether signal has arrived
            if ( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received." );
                return erOK;
            }

            if ( FD_ISSET( server.GetSocket(), &readset ) )
            {
                m_ServerCocket = server.Accept();

                // executing PF threads
                m_thrd_clnt = Thread_PTR_t( new boost::thread(
                                                boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerCocket, &m_ClientSocket ) ) );
                m_thrd_srv = Thread_PTR_t( new boost::thread(
                                               boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ClientSocket, &m_ServerCocket ) ) );
                break;
            }
        }

        if ( _Join )
        {
            m_thrd_clnt->join();
            m_thrd_srv->join();
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
        return erError;
    }
    return erOK;
}

void CPacketForwarder::WriteBuffer( BYTEVector_t &_Buf, smart_socket &_socket ) throw ( exception )
{
    //  boost::mutex::scoped_lock lock(m_Buf_mutex);
    _socket << _Buf;
    ReportPackage( _socket, _Buf, false );
}
