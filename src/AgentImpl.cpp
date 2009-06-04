/************************************************************************/
/**
 * @file AgentImpl.cpp
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
// PROOFAgent
#include "AgentImpl.h"
#include "INet.h"
#include "PARes.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;
//=============================================================================
const size_t g_READ_READY_INTERVAL = 4;

sig_atomic_t graceful_quit = 0;
//=============================================================================
void PROOFAgent::signal_handler( int _SignalNumber )
{
    graceful_quit = 1;
}
//=============================================================================
//------------------------- Agent SERVER ------------------------------------------------------------
void CAgentServer::ThreadWorker()
{
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( m_sPROOFCfg );
    try
    {
        CSocketServer server;
        server.Bind( m_Data.m_nPort );
        server.Listen( 100 ); // TODO: Move this number of queued clients to config
        server.GetSocket().set_nonblock(); // Nonblocking server socket
        while ( true )
        {
            // TODO: we need to check real PROOF port here (from cfg)
            if ( !IsPROOFReady( 0 ) )
            {
                FaultLog( erError, "Can't connect to PROOF/XRD service." );
                graceful_quit = 1;
            }

            // Checking whether signal has arrived
            if ( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received." );
                return ;
            }

            if ( server.GetSocket().is_read_ready( g_READ_READY_INTERVAL ) )
            {
                smart_socket socket( server.Accept() );

                // checking protocol version
                string sReceive;
                receive_string( socket, &sReceive, g_nBUF_SIZE );
                DebugLog( erOK, "Server received: " + sReceive );

                // TODO: Implement protocol version check
                string sOK( g_szRESPONSE_OK );
                DebugLog( erOK, "Server sends: " + sOK );
                send_string( socket, sOK );

                // TODO: Receiving user name -- now we always assume that this is a user name -- WE NEED to implement a simple protocol!!!
                string sUsrName;
                receive_string( socket, &sUsrName, g_nBUF_SIZE );
                DebugLog( erOK, "Server received: " + sUsrName );

                DebugLog( erOK, "Server sends: " + sOK );
                send_string( socket, sOK );

                string strSocketInfo;
                socket2string( socket, &strSocketInfo );
                string strSocketPeerInfo;
                peer2string( socket, &strSocketPeerInfo );
                stringstream ss;
                ss
                << "Accepting connection on : " << strSocketInfo
                << " for peer: " << strSocketPeerInfo;
                InfoLog( erOK, ss.str() );

                const int port = get_free_port( m_Data.m_nLocalClientPortMin, m_Data.m_nLocalClientPortMax );
                if ( 0 == port )
                    throw runtime_error( "Can't find any free port from the given range." );

                // Add a worker to PROOF cfg
                string strRealWrkHost;
                string strPROOFCfgString;
                peer2string( socket, &strRealWrkHost );
                AddWrk2PROOFCfg( m_sPROOFCfg, sUsrName, port, strRealWrkHost, &strPROOFCfgString );

                // Spawn PortForwarder
                AddPF( socket.detach(), port, strPROOFCfgString );
            }
            // TODO: Needs to be optimized. Maybe moved to a different thread
            // cleaning all PF which are in disconnect state
            CleanDisconnectsPF( m_sPROOFCfg );
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
//=============================================================================
//------------------------- Agent CLIENT ------------------------------------------------------------
void CAgentClient::ThreadWorker()
{
    DebugLog( erOK, "Starting main thread..." );
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( m_sPROOFCfg );
    CSocketClient client;
    try
    {
        while ( true )
        {
            if ( !IsPROOFReady( m_Data.m_nLocalClientPort ) )
            {
                FaultLog( erError, "Can't connect to PROOF/XRD service." );
                graceful_quit = 1;
                break;
            }

            DebugLog( erOK, "looking for PROOFAgent server to connect..." );
            CSocketClient client;
            client.Connect( m_Data.m_nServerPort, m_Data.m_strServerHost );
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


            // Spawn PortForwarder
            CPacketForwarder pf( client.GetSocket(), m_Data.m_nLocalClientPort );
            pf.Start( true, m_Data.m_shutdownIfIdleForSec );
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
