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
sig_atomic_t shutdown_client = 0;
//=============================================================================
void PROOFAgent::signal_handler( int _SignalNumber )
{
    graceful_quit = 1;
}
//=============================================================================
//------------------------- Agent Base class --------------------------------------------------------
void CAgentBase::readServerInfoFile( const string &_filename )
{
    boost::program_options::variables_map keys;
    boost::program_options::options_description options( "Agent's server info config" );
    // HACK: Don't make a long add_options, otherwise Eclipse 3.5's CDT idexer can't handle it
    options.add_options()
    ( "server.host", boost::program_options::value<std::string>(), "" )
    ( "server.port", boost::program_options::value<unsigned int>(), "" )
    ;
    std::ifstream ifs( _filename.c_str() );
    if ( !ifs.is_open() || !ifs.good() )
    {
        string msg( "Could not open a server info configuration file: " );
        msg += _filename;
        throw runtime_error( msg );
    }
    // Parse the config file
    boost::program_options::store( boost::program_options::parse_config_file( ifs, options ), keys );
    boost::program_options::notify( keys );
    if ( keys.count("server.host") )
        m_agentServerHost = keys["server.host"].as<string>();
    if ( keys.count("server.port") )
        m_agentServerListenPort = keys["server.port"].as<unsigned int>();
}
//=============================================================================
//------------------------- Agent SERVER ------------------------------------------------------------
void CAgentServer::ThreadWorker()
{
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( m_commonOptions.m_proofCFG );
    try
    {
        readServerInfoFile( m_serverInfoFile );

        CSocketServer server;
        server.Bind( m_agentServerListenPort );
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

                const int port = get_free_port( m_Data.m_agentLocalClientPortMin, m_Data.m_agentLocalClientPortMax );
                if ( 0 == port )
                    throw runtime_error( "Can't find any free port from the given range." );

                // Add a worker to PROOF cfg
                string strRealWrkHost;
                string strPROOFCfgString;
                peer2string( socket, &strRealWrkHost );
                AddWrk2PROOFCfg( m_commonOptions.m_proofCFG, sUsrName, port, strRealWrkHost, &strPROOFCfgString );

                // Spawn PortForwarder
                AddPF( socket.detach(), port, strPROOFCfgString );
            }
            // TODO: Needs to be optimized. Maybe moved to a different thread
            // cleaning all PF which are in disconnect state
	    CleanDisconnectsPF( m_commonOptions.m_proofCFG );
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
//=============================================================================
void CAgentServer::deleteServerInfoFile()
{
    // TODO: check error code
    unlink( m_serverInfoFile.c_str() );
}
//=============================================================================
//------------------------- Agent CLIENT ------------------------------------------------------------
void CAgentClient::ThreadWorker()
{
    DebugLog( erOK, "Starting main thread..." );
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( m_commonOptions.m_proofCFG );
    CSocketClient client;
    try
    {
        while ( true )
        {
            // worker needs to be closed
            if ( shutdown_client )
                break;

            if ( !IsPROOFReady( m_proofPort ) )
            {
                FaultLog( erError, "Can't connect to PROOF/XRD service." );
                graceful_quit = 1;
                break;
            }

            readServerInfoFile( m_serverInfoFile );

            DebugLog( erOK, "looking for PROOFAgent server to connect..." );
            CSocketClient client;
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


            // Spawn PortForwarder
            CPacketForwarder pf( client.GetSocket(), m_proofPort );
            pf.Start( true, m_Data.m_shutdownIfIdleForSec );
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
