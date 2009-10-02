/************************************************************************/
/**
 * @file AgentServer.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "ErrorCode.h"
#include "INet.h"
// PROOFAgent
#include "AgentServer.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
namespace inet = MiscCommon::INet;
//=============================================================================
const size_t g_READ_READY_INTERVAL = 4;
extern sig_atomic_t graceful_quit;
//=============================================================================
namespace PROOFAgent
{

//=============================================================================
    CAgentServer::CAgentServer( const SOptions_t &_data ): CAgentBase( _data.m_podOptions.m_server.m_common )
    {
        m_Data = _data.m_podOptions.m_server;
        m_serverInfoFile = _data.m_serverInfoFile;

        //InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
    }

//=============================================================================
    CAgentServer::~CAgentServer()
    {
        deleteServerInfoFile();
    }

//=============================================================================
    void CAgentServer::AddPF( inet::Socket_t _ClientSocket,
                unsigned short _nNewLocalPort,
                const string &_sPROOFCfgString )
    {
        boost::mutex::scoped_lock lock( m_PFList_mutex );
        m_PFList.add( _ClientSocket, _nNewLocalPort, _sPROOFCfgString );
    }

//=============================================================================
    void CAgentServer::CleanDisconnectsPF( const string &_sPROOFCfg )
    {
        m_PFList.clean_disconnects( _sPROOFCfg );
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

            inet::CSocketServer server;
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
                    inet::smart_socket socket( server.Accept() );

                    // checking protocol version
                    string sReceive;
                    inet::receive_string( socket, &sReceive, g_nBUF_SIZE );
                    DebugLog( erOK, "Server received: " + sReceive );

                    // TODO: Implement protocol version check
                    string sOK( g_szRESPONSE_OK );
                    DebugLog( erOK, "Server sends: " + sOK );
                    inet::send_string( socket, sOK );

                    // TODO: Receiving user name -- now we always assume that this is a user name -- WE NEED to implement a simple protocol!!!
                    string sUsrName;
                    receive_string( socket, &sUsrName, g_nBUF_SIZE );
                    DebugLog( erOK, "Server received: " + sUsrName );

                    DebugLog( erOK, "Server sends: " + sOK );
                    send_string( socket, sOK );

                    string strSocketInfo;
                    inet::socket2string( socket, &strSocketInfo );
                    string strSocketPeerInfo;
                    inet::peer2string( socket, &strSocketPeerInfo );
                    stringstream ss;
                    ss
                    << "Accepting connection on : " << strSocketInfo
                    << " for peer: " << strSocketPeerInfo;
                    InfoLog( erOK, ss.str() );

                    const int port = inet::get_free_port( m_Data.m_agentLocalClientPortMin, m_Data.m_agentLocalClientPortMax );
                    if ( 0 == port )
                        throw runtime_error( "Can't find any free port from the given range." );

                    // Add a worker to PROOF cfg
                    string strRealWrkHost;
                    string strPROOFCfgString;
                    inet::peer2string( socket, &strRealWrkHost );
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
}
