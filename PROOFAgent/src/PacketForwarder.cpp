/************************************************************************/
/**
 * @file PacketForwarder.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
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

// a regular Ethernet frame size - datagram
// TODO: Move it to config.
const unsigned int g_BUF_SIZE = 1500;

extern sig_atomic_t graceful_quit;

bool CPacketForwarder::ForwardBuf( smart_socket *_Input, smart_socket *_Output )
{ // TODO: Do we need to optimize and use an external buffer (from the higher scope)?
    if ( _Input->is_read_ready( 2 ) )
    {
        boost::mutex::scoped_lock lock(m_mutex);
        BYTEVector_t buf( g_BUF_SIZE );

        *_Input >> &buf;

        // DISCONNECT has been detected
        if ( !_Input->is_valid() )
            return false;

        *_Output << buf;

        ReportPackage( *_Input, *_Output, buf );
    }
    return true;
}

void CPacketForwarder::ThreadWorker( smart_socket *_SrvSocket, smart_socket *_CltSocket )
{
    while ( true )
    {
        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return ;
        }

        try
        {
            if ( !ForwardBuf( _SrvSocket, _CltSocket ) )
            {
                InfoLog( erOK, "DISCONNECT has been detected." );
                _SrvSocket->close();
                _CltSocket->close();
                return ;
            }
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
            return ;
        }
    }
}

ERRORCODE CPacketForwarder::Start( bool _ClientMode )
{
    // executing PF threads
    m_thrd_serversocket = Thread_PTR_t( new boost::thread(
                                            boost::bind( &CPacketForwarder::_Start, this, _ClientMode ) ) );
    if ( _ClientMode )  //  Join Threads (for Client) and non-join Threads (for Server mode - server shouldn't sleep while PF is working)
        m_thrd_serversocket->join();

    return erOK;
}

void CPacketForwarder::SpawnClientMode()
{
    m_ClientSocket.set_nonblock();
    while (true)
    {
        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return ;
        }

        try
        {
            if ( m_ClientSocket.is_read_ready( 10 ) )
                break;
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
            return ;
        }
    }
    CSocketClient proof_client;
    proof_client.Connect( m_nPort, "127.0.0.1" );

    // Checking whether signal has arrived
    if ( graceful_quit )
    {
        InfoLog( erOK, "STOP signal received." );
        return ;
    }
    m_ServerSocket = proof_client.GetSocket().detach();

    m_ServerSocket.set_nonblock();

    // executing PF threads
    m_thrd_clnt = Thread_PTR_t( new boost::thread(
                                    boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerSocket, &m_ClientSocket) ) );
    m_thrd_srv = Thread_PTR_t( new boost::thread(
                                   boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ClientSocket, &m_ServerSocket) ) );

    m_thrd_clnt->join();
    m_thrd_srv->join();

    // Closing client when server disconnects and closing server when xrootd redirector disconnects
    //   if (m_Disc)
    //       graceful_quit = true;
}

void CPacketForwarder::SpawnServerMode()
{
    while ( true )
    {
        CSocketServer server;
        server.Bind( m_nPort );
        server.Listen( 1 );
        server.GetSocket().set_nonblock();

        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return ;
        }

        if ( server.GetSocket().is_read_ready( 10 ) )
        {
            m_ServerSocket = server.Accept();

            // executing PF threads
            m_thrd_clnt = Thread_PTR_t( new boost::thread(
                                            boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerSocket, &m_ClientSocket ) ) );
            m_thrd_srv = Thread_PTR_t( new boost::thread(
                                           boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ClientSocket, &m_ServerSocket ) ) );
            break;
        }
    }
}


ERRORCODE CPacketForwarder::_Start( bool _ClientMode )
{
    try
    {
        if ( _ClientMode )
            SpawnClientMode();
        else
            SpawnServerMode();
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
        return erError;
    }
    return erOK;
}
