/************************************************************************/
/**
 * @file AgentImpl.cpp
 * @brief Implementation file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// API
#include <sys/types.h>

// XML parser
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

// STD
#include <stdexcept>

// PROOFAgent
#include "XMLHelper.h"
#include "AgentImpl.h"
#include "INet.h"
#include "PARes.h"

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
using namespace MiscCommon::INet;
XERCES_CPP_NAMESPACE_USE;
using namespace PROOFAgent;


sig_atomic_t graceful_quit = 0;

void PROOFAgent::signal_handler( int _SignalNumber )
{
    graceful_quit = 1;
}

//------------------------- Agent SERVER ------------------------------------------------------------
ERRORCODE CAgentServer::Read( DOMNode* _element )
{
    // TODO: Use a "try" block to catch XML exceptions
    if ( NULL == _element )
        throw( invalid_argument( "DOME Node object is NULL" ) );

    // getting <agent_server> Element
    DOMNode* node = GetSingleNodeByName_Ex( _element, "agent_server" );

    DOMElement* elementConfig( NULL );
    if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
        elementConfig = dynamic_cast< DOMElement* >( node ) ;
    if ( NULL == elementConfig )
        throw( runtime_error( "empty XML document" ) );

    // retrieving attributes
    get_attr_value( elementConfig, "listen_port", &m_Data.m_nPort );
    get_attr_value( elementConfig, "local_client_port_min", &m_Data.m_nLocalClientPortMin );
    get_attr_value( elementConfig, "local_client_port_max", &m_Data.m_nLocalClientPortMax );

    InfoLog( erOK, "Agent Server configuration:" ) << m_Data;

    return erOK;
}

ERRORCODE CAgentServer::Write( DOMNode* _element )
{
    return erNotImpl;
}

void CAgentServer::ThreadWorker( const std::string &_PROOFCfg, const std::string &_WorkDir )
{
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( _PROOFCfg );
    try
    {
        CSocketServer server;
        server.Bind( m_Data.m_nPort );
        server.Listen( 10 ); // TODO: Move this number of queued clients to config
        server.GetSocket().set_nonblock(); // Nonblocking server socket
        while ( true )
        {
            // Checking whether signal has arrived
            if ( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received." );
                return ;
            }

            if ( server.GetSocket().is_read_ready( 10 ) )
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
                    throw runtime_error("Can't find any free port from the given range.");

                // Add a worker to PROOF cfg
                AddWrk2PROOFCfg( _PROOFCfg, _WorkDir, sUsrName, port );

                // Spwan PortForwarder
                Socket_t s = socket.detach();
                AddPF( s, port );
            }
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}


//------------------------- Agent CLIENT ------------------------------------------------------------
ERRORCODE CAgentClient::Read( DOMNode* _element )
{
    // TODO: Use a "try" block to catch XML exceptions
    if ( NULL == _element )
        return erXMLNullNode;

    // getting <agent_server> Element
    DOMNode* node = GetSingleNodeByName_Ex( _element, "agent_client" );

    DOMElement* elementConfig( NULL );
    if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
        elementConfig = dynamic_cast< DOMElement* >( node ) ;
    if ( NULL == elementConfig )
        throw( runtime_error( "empty XML document" ) );

    // retrieving attributes
    get_attr_value( elementConfig, "server_port", &m_Data.m_nServerPort );
    get_attr_value( elementConfig, "server_addr", &m_Data.m_strServerHost );
    get_attr_value( elementConfig, "local_proofd_port", &m_Data.m_nLocalClientPort );

    InfoLog( erOK, "Agent Client configuration:" ) << m_Data;

    return erOK;
}

ERRORCODE CAgentClient::Write( DOMNode* _element )
{
    return erNotImpl;
}

void CAgentClient::ThreadWorker( const std::string &_PROOFCfg, const std::string &_WorkDir )
{
    DebugLog( erOK, "Starting main thread..." );
    DebugLog( erOK, "Creating a PROOF configuration file..." );
    CreatePROOFCfg( _PROOFCfg );
    CSocketClient client;
    try
    {
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
        pf.Start( true );
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
