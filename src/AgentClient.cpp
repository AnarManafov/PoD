/************************************************************************/
/**
 * @file AgentClient.cpp
 * @brief Implementation file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// API
#include <sys/types.h>
// STD
#include <csignal>
// PROOFAgent
#include "Node.h"
#include "PARes.h"
#include "AgentClient.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
namespace inet = MiscCommon::INet;
//=============================================================================
const size_t g_monitorTimeout = 10; // in seconds
extern sig_atomic_t graceful_quit;
//=============================================================================
void CAgentClient::run()
{
    // DebugLog( erOK, "Creating a PROOF configuration file..." );
    // TODO: Implement createPROOFCfg for the client mode
    //CreatePROOFCfg( m_commonOptions.m_proofCFG );

    try
    {
        while ( true )
        {
            if ( !IsPROOFReady( m_proofPort ) )
            {
                FaultLog( erError, "Can't connect to PROOF/XRD service." );
                graceful_quit = 1;
                break;
            }

            readServerInfoFile( m_serverInfoFile );

            DebugLog( erOK, "looking for PROOFAgent server to connect..." );
            // a connection to a Agent's server
            inet::CSocketClient client;
            client.GetSocket().set_nonblock();
            client.Connect( m_agentServerListenPort, m_agentServerHost );
            DebugLog( erOK, "connected!" );

            // sending protocol version to the server
            string sProtocol( g_szPROTOCOL_VERSION );
            DebugLog( erOK, "Sending protocol version: " + sProtocol );
            send_string( client.GetSocket(), sProtocol );

            //TODO: Checking response
            string sResponse;
            receive_string( client.GetSocket(), &sResponse, g_nBUF_SIZE );
            DebugLog( erOK, "Client received: " + sResponse );

            // Sending User name
            string sUser;
            get_cuser_name( &sUser );
            DebugLog( erOK, "Sending user name: " + sUser );
            send_string( client.GetSocket(), sUser );

            //TODO: Checking response
            receive_string( client.GetSocket(), &sResponse, g_nBUF_SIZE );
            DebugLog( erOK, "Client received: " + sResponse );

            InfoLog( "Entering into the main \"select\" loop..." );

            try
            {
                // waiting until Agent server sends something
                // that would mean that a user initializes a PROOF session
                waitForServerToConnect( client.GetSocket().get() );

                // now we need to connect to a local PROOF worker
                MiscCommon::INet::Socket_t proof_socket = connectToLocalPROOF( m_proofPort );
                CNode node( client.GetSocket().detach(), proof_socket,
                            "", m_Data.m_common.m_agentNodeReadBuffer );
                // now we are ready to proxy all packages
                mainSelect( &node );
            }
            catch ( exception & e )
            {
                FaultLog( erError, e.what() );
                continue;
            }
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}

//=============================================================================
void CAgentClient::monitor()
{
    while ( true )
    {
        // TODO: we need to check real PROOF port here (from cfg)
        if ( !IsPROOFReady( m_proofPort ) || graceful_quit  )
        {
            FaultLog( erError, "Can't connect to PROOF/XRD service." );
            graceful_quit = 1;

            // wake up (from "select") the main thread, so that it can update it self
            if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
                FaultLog( erError, "Can't signal to the main thread via a named pipe: " + errno2str() );

            return;
        }

        sleep( g_monitorTimeout );
    }
}

//=============================================================================
void CAgentClient::waitForServerToConnect( MiscCommon::INet::Socket_t _sockToWait )
{
    fd_set readset;
    FD_ZERO( &readset );
    FD_SET( _sockToWait, &readset );
    // setting a signal pipe as well
    // we want to be interrupted
    FD_SET( m_fdSignalPipe, &readset );

    int retval = ::select(( _sockToWait > m_fdSignalPipe ? _sockToWait : m_fdSignalPipe ) + 1,
                          &readset, NULL, NULL, NULL );
    if ( retval < 0 )
        throw system_error( "Error occurred while waiting for Agent server:" );

    // must actually never happen
    if ( 0 == retval )
        throw system_error( "Select has timeout while waiting for an Agent server." );

    // we got a signal for update
    // reading everything from the pipe and exiting from the function
    if ( FD_ISSET( m_fdSignalPipe, &readset ) )
    {
        const int read_size = 20;
        char buf[read_size];
        int numread( 0 );
        do
        {
            numread = read( m_fdSignalPipe, buf, read_size );
        }
        while ( numread > 0 );

        throw system_error( "Got a wake up signal from the signal pipe." );
    }

    m_idleWatch.touch();

    // if there were no exception raised we can process further
}

//=============================================================================
MiscCommon::INet::Socket_t CAgentClient::connectToLocalPROOF( unsigned int _proofPort )
{
    // Connecting to the local client (a proof slave)
    inet::CSocketClient proof_client;
    proof_client.Connect( _proofPort, "127.0.0.1" );
    InfoLog( erOK, "connected to the local proof service" );

    proof_client.GetSocket().set_nonblock();

    return proof_client.GetSocket().detach();
}

//=============================================================================
void CAgentClient::mainSelect( CNode *_node )
{
    while ( true )
    {
    	m_idleWatch.touch();

        // Checking whether signal has arrived
        if ( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return ;
        }


        fd_set readset;
        FD_ZERO( &readset );
        FD_SET( _node->first(), &readset );
        FD_SET( _node->second(), &readset );
        int fd_max( _node->first() > _node->second() ? _node->first() : _node->second() );
        // setting a signal pipe as well
        // we want to be interrupted
        FD_SET( m_fdSignalPipe, &readset );
        fd_max = fd_max > m_fdSignalPipe ? fd_max : m_fdSignalPipe;


        int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );
        if ( retval < 0 )
            throw system_error( "Error occurred while in the main select:" );

        // must actually never happen
        if ( 0 == retval )
            throw system_error( "The main select has timeout." );

        if ( FD_ISSET( _node->first(), &readset ) )
        {
            _node->dealWithData( _node->first() );
        }
        if ( FD_ISSET( _node->second(), &readset ) )
        {
            _node->dealWithData( _node->second() );
        }


        // we got a signal for update
        // reading everything from the pipe and exiting from the function
        if ( FD_ISSET( m_fdSignalPipe, &readset ) )
        {
            const int read_size = 20;
            char buf[read_size];
            int numread( 0 );
            do
            {
                numread = read( m_fdSignalPipe, buf, read_size );
            }
            while ( numread > 0 );

            throw system_error( "Got a wake up signal from the signal pipe." );
        }

    }
}
