/************************************************************************/
/**
 * @file PacketForwarder.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
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
namespace boost_hlp = MiscCommon::BOOSTHelper;

// a number of seconds, which is define an interval for select function
const size_t g_CHECK_INTERVAL = 2;
const size_t g_CHECK_SERVERMSG_INTERVAL = 3;
const size_t g_SERVER_INTERVAL = 3;
// a regular Ethernet frame size - datagram
// TODO: Move it to config.
const unsigned int g_BUF_SIZE = 1500;

extern sig_atomic_t graceful_quit;

bool CPacketForwarder::ForwardBuf( smart_socket *_Input, smart_socket *_Output )
{
    // DISCONNECT has been detected
    if ( !_Output->is_valid() || !_Input->is_valid() )
        return false;

    fd_set readset;
    FD_ZERO( &readset );

    FD_SET( _Input->get(), &readset );
    FD_SET( _Output->get(), &readset );

    // Setting time-out
    timeval timeout;
    timeout.tv_sec = g_CHECK_INTERVAL;
    timeout.tv_usec = 0;

    int fd_max = max( _Input->get(), _Output->get() );
    // TODO: Send errno to log
    int retval = ::select( fd_max + 1, &readset, NULL, NULL, &timeout );

    if ( retval < 0 )
        throw std::runtime_error( "Server socket got error while calling \"select\"" );

    if ( 0 == retval )
        return true;

    smart_socket *readSock = NULL;
    smart_socket *writeSock = NULL;

    if ( FD_ISSET( _Input->get(), &readset ) )
    {
        readSock = _Input;
        writeSock = _Output;
    }
    else
    {
        readSock = _Output;
        writeSock = _Input;
    }

    {
        boost::mutex::scoped_lock lock( m_mutex );
        BYTEVector_t buf( g_BUF_SIZE );
        *readSock >> &buf;

        // DISCONNECT has been detected
        if ( !_Output->is_valid() || !_Input->is_valid() )
            return false;

        *writeSock << buf;

        ReportPackage( *readSock, *writeSock, buf );
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
            InfoLog( erOK, "STOP signal received on PF worker thread." );
            break;
        }

        try
        {
            if ( !ForwardBuf( _SrvSocket, _CltSocket ) )
            {
                InfoLog( erOK, "DISCONNECT has been detected on PF worker thread." );
                break;
            }
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
            break;
        }
    }

    InfoLog( erOK, "Forcing PF sockets to close..." );
    m_ClientSocket.close();
    m_ServerSocket.close();
    InfoLog( erOK, "PF sockets are closed." );
}

ERRORCODE CPacketForwarder::Start( bool _ClientMode )
{
    // executing PF threads
    m_thrd_serversocket = boost_hlp::Thread_PTR_t( new boost::thread(
                                                       boost::bind( &CPacketForwarder::_Start, this, _ClientMode ) ) );
    //  Join Threads (for Client) and non-join Threads for Server mode - server shouldn't sleep while PF is working.
    if ( _ClientMode )
        m_thrd_serversocket->join();

    return erOK;
}

void CPacketForwarder::SpawnClientMode()
{
    m_ClientSocket.set_nonblock();
    // Waiting a server peer for data
    // PROOF server connects to its client (to us), not other way around.
    while ( true )
    {
        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received on the Client mode." );
            return ;
        }

        try
        {
            if ( m_ClientSocket.is_read_ready( g_CHECK_SERVERMSG_INTERVAL ) )
                break; // there is something from the server arrived
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
            return ;
        }
    }
    // Connecting to the local client (a proof slave)
    CSocketClient proof_client;
    proof_client.Connect( m_nPort, "127.0.0.1" );

    // Checking whether signal has arrived
    if ( graceful_quit )
    {
        InfoLog( erOK, "STOP signal received on the Client mode." );
        return ;
    }
    m_ServerSocket = proof_client.GetSocket().detach();

    m_ServerSocket.set_nonblock();

    // executing PF threads
//   m_thrd_pf = boost_hlp::Thread_PTR_t( new boost::thread(
//                                             boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerSocket, &m_ClientSocket ) ) );
    // in the Client mode we wait for the threads
//   m_thrd_pf->join();

    // Executing PF routine
    ThreadWorker( &m_ServerSocket, &m_ClientSocket );
}

void CPacketForwarder::SpawnServerMode()
{
    while ( true )
    {
        // Listening for PROOF master connections
        // Whenever he tries to connect to its clients we will catch it and redirect it
        CSocketServer server;
        server.Bind( m_nPort );
        server.Listen( 1 );
        server.GetSocket().set_nonblock();

        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received on the Server mode." );
            return ;
        }

        if ( server.GetSocket().is_read_ready( g_SERVER_INTERVAL ) )
        {
            // A PROOF master connection
            m_ServerSocket = server.Accept();

            // executing PF threads
//           m_thrd_pf = boost_hlp::Thread_PTR_t( new boost::thread(
//                                                    boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerSocket, &m_ClientSocket ) ) );
            break;
        }
    }

    // Executing PF routine
    ThreadWorker( &m_ServerSocket, &m_ClientSocket );
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
