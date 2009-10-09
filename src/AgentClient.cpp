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
#include "AgentClient.h"
#include "PARes.h"
//#include "NewPacketForwarder.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;
//=============================================================================
const size_t g_monitorTimeout = 5; // in seconds
extern sig_atomic_t graceful_quit;
sig_atomic_t shutdown_client = 0;
//=============================================================================
void CAgentClient::run()
{
    DebugLog( erOK, "Starting main thread..." );
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    // TODO: Implement createPROOFCfg for the client mode
    //CreatePROOFCfg( m_commonOptions.m_proofCFG );
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
            pf.Start( true, m_Data.m_common.m_shutdownIfIdleForSec );
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

//    	if ( shutdown_client )
//    	                break;

        // TODO: we need to check real PROOF port here (from cfg)
//        if ( !IsPROOFReady( m_proofPort )  )
//        {
//            FaultLog( erError, "Can't connect to PROOF/XRD service." );
//            graceful_quit = 1;
//
//            // wake up (from "select") the main thread, so that it can update it self
//            if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
//                FaultLog( erError, "Can't signal to the main thread via a named pipe: " + errno2str() );
//
//            return;
//        }

        sleep( g_monitorTimeout );
    }
}

