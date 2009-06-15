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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
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
extern sig_atomic_t shutdown_client;

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
    else if ( FD_ISSET( _Output->get(), &readset ) )
    {
        readSock = _Output;
        writeSock = _Input;
    }
    else 
      return true;

    m_idleWatch.touch();

    {
        boost::mutex::scoped_lock lock( m_mutex );
        BYTEVector_t buf( g_BUF_SIZE );
	InfoLog( erOK, "there is a message to redirect." );
        *readSock >> &buf;

        // DISCONNECT has been detected
        if ( !_Output->is_valid() || !_Input->is_valid() )
            return false;

	InfoLog( erOK, "redirecting the message..." );
        *writeSock << buf;

        ReportPackage( *readSock, *writeSock, buf );
    }
    return true;
}

void CPacketForwarder::ThreadWorker( smart_socket *_SrvSocket, smart_socket *_CltSocket )
{
    m_idleWatch.touch();

    while ( true )
    {
        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received on PF worker thread." );
            break;
        }

        if ( m_idleWatch.isTimedout( m_shutdownIfIdleForSec ) )
        {
            InfoLog( erOK, "PF reached an idle timeout. Exiting..." );
            graceful_quit = 1;
	    shutdown_client = 1;
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

ERRORCODE CPacketForwarder::Start( bool _ClientMode, int _shutdownIfIdleForSec )
{
    m_shutdownIfIdleForSec = _shutdownIfIdleForSec;
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
    m_idleWatch.touch();

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
        if ( m_idleWatch.isTimedout( m_shutdownIfIdleForSec ) )
        {
            InfoLog( erOK, "PF reached an idle timeout. Exiting..." );
	    graceful_quit = 1;
	    shutdown_client = 1;
            return;
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
    InfoLog( erOK, "connecting to a local proof service on port: " ) << m_nPort << endl;
    proof_client.Connect( m_nPort, "127.0.0.1" );
    InfoLog( erOK, "connected to the local proof service" );

    // Checking whether signal has arrived
    if ( graceful_quit )
    {
        InfoLog( erOK, "STOP signal received on the Client mode." );
        return ;
    }
    m_ServerSocket = proof_client.GetSocket().detach();

    m_ServerSocket.set_nonblock();

    InfoLog( erOK, "starting PF routine..." );
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

	// Check whether worker has disconnected (m_ClientSocket)
	// and also check that a user (or better to say xrootd redirector) is connecting to a PROOF cluster (server.GetSocket())
	fd_set readset;
	FD_ZERO( &readset );

	FD_SET( server.GetSocket().get(), &readset );
	FD_SET( m_ClientSocket.get(), &readset );

	// Setting time-out
	timeval timeout;
	timeout.tv_sec = g_SERVER_INTERVAL;
	timeout.tv_usec = 0;
	
	int fd_max = max( server.GetSocket().get(), m_ClientSocket.get() );
	// TODO: Send errno to log
	int retval = ::select( fd_max + 1, &readset, NULL, NULL, &timeout );
	if ( retval < 0 )
		throw std::runtime_error( "Server socket got error while calling \"select\"" );
	     
	if ( FD_ISSET( server.GetSocket().get(), &readset ) )
	{
		// A PROOF master connection
		m_ServerSocket = server.Accept();
		break;
	}
	else if ( FD_ISSET( m_ClientSocket.get(), &readset ) )
	{
		// worker is sending something, but we don't expect anything
		// in this version of protocol it means, that the workers disconnects
		// TODO: make check for the size of data to read (ioctl). if the size is 0, then it is a dissconnect
		InfoLog( erOK, "A DISCONNECT has been detected of a PA worker. The worker will be removed from the active list." );
		m_ClientSocket.close();
		return;
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
