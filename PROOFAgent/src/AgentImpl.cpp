/************************************************************************/
/**
 * @file AgentImpl.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                     2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
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
        return erXMLNullNode;

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

void CAgentServer::ThreadWorker()
{
    try
    {
        CSocketServer server;
        server.Bind( m_Data.m_nPort );
        server.Listen( 10 ); // TODO: Move this number of queued clients to config
        server.GetSocket().set_nonblock(); // Nonblocking server socket
        fd_set readset;
        while ( true )
        {
            FD_ZERO( &readset );
            FD_SET( server.GetSocket(), &readset );
            // Setting time-out
            timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            if ( ::select( server.GetSocket() + 1, &readset, NULL, NULL, &timeout ) < 0 )
            { // TODO: Send errno to log
                FaultLog( erError, "Server socket got error while calling \"select\"" );
                return ;
            }

            // Checking whether signal has arrived
            if ( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received." );
                return ;
            }

            if ( FD_ISSET( server.GetSocket(), &readset ) )
            {
                smart_socket socket( server.Accept() );

                //                 { // a transfer test
                //                     BYTEVector_t buf ( 1024 );
                //                     socket >> &buf;
                //                     m_pThis->LogThread( "Server recieved: " + string( reinterpret_cast<char*>( &buf[ 0 ] ) ) );
                //                 }
                // TODO: recieve data from client here
                string strSocketInfo;
                socket2string( socket, &strSocketInfo );
                string strSocketPeerInfo;
                peer2string( socket, &strSocketPeerInfo );
                stringstream ss;
                ss
                << "Accepting connection on : " << strSocketInfo
                << " for peer: " << strSocketPeerInfo;
                InfoLog( erOK, ss.str() );

                static unsigned short port = m_Data.m_nLocalClientPortMin; // TODO: Implement port enumerator - should give next free port by requests
                // Spwan PortForwarder
                Socket_t s = socket.detach();
                AddPF( s, ++port );
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
    get_attr_value( elementConfig, "local_client_port", &m_Data.m_nLocalClientPort );

    InfoLog( erOK, "Agent Client configuration:" ) << m_Data;

    return erOK;
}

ERRORCODE CAgentClient::Write( DOMNode* _element )
{
    return erNotImpl;
}

void CAgentClient::ThreadWorker()
{
    DebugLog( erOK, "Starting main thread..." );
    CSocketClient client;
    try
    {
        CSocketClient client;
        client.Connect( m_Data.m_nServerPort, m_Data.m_strServerHost );
        DebugLog( erOK, "connected!" );

        //             { // a transfer test
        //                 string sTest( "Test String!" );
        //                 BYTEVector_t buf;
        //                 copy( sTest.begin(), sTest.end(), back_inserter( buf ) );
        //                 m_pThis->LogThread( "Sending: " + string( ( char* ) & buf[ 0 ] ) );
        //                 client.GetSocket() << buf;
        //             }

        // Spwan PortForwarder
        CPacketForwarder pf( client.GetSocket(), m_Data.m_nLocalClientPort );
        pf.Start( true );
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
