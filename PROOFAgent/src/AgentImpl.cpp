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

const unsigned short NEW_PORT = 51511;

//------------------------- Agent SERVER ------------------------------------------------------------
ERRORCODE CAgentServer::Read( DOMNode* _element )
{
    // TODO: Use a "try" block to catch XML exceptions
    if ( NULL == _element )
        return erXMLNullNode;

    // getting <agent_server> Element
    smart_XMLCh ElementName( "agent_server" );
    DOMElement *config_element( dynamic_cast<DOMElement* >( _element ) );
    if ( NULL == config_element )
        return erXMLNullNode;

    DOMNodeList *list = config_element->getElementsByTagName( ElementName );

    DOMNode* node = list->item( 0 ) ;
    DOMElement* elementConfig( NULL );
    if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
        elementConfig = dynamic_cast< DOMElement* >( node ) ;

    if ( NULL == elementConfig )
        throw( runtime_error( "empty XML document" ) );

    // retrieving attributs
    get_attr_value( elementConfig, "listen_port", &m_Data.m_nPort );

    InfoLog( erOK, "Agent Server configuration:" ) << m_Data;

    return erOK;
}

ERRORCODE CAgentServer::Write( xercesc::DOMNode* _element )
{
    return erNotImpl;
}

void CAgentServer::ThreadWorker()
{
    try
    {
        CSocketServer server;
        server.Bind( m_Data.m_nPort );
        server.Listen( 1 );
        while ( true )
        {
            smart_socket socket( server.Accept() );

            //                 { // a transfer test
            //                     BYTEVector_t buf ( 1024 );
            //                     socket >> &buf;
            //                     m_pThis->LogThread( "Server recieved: " + string( reinterpret_cast<char*>( &buf[ 0 ] ) ) );
            //                 }
            // TODO: recieve data from client here

            static unsigned short port = NEW_PORT;
            // Spwan PortForwarder
            Socket_t s = socket.deattach();
            AddPF( s, ++port );
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
    smart_XMLCh ElementName( "agent_client" );
    DOMElement *config_element( dynamic_cast<DOMElement* >( _element ) );
    if ( NULL == config_element )
        return erXMLNullNode;

    DOMNodeList *list = config_element->getElementsByTagName( ElementName );

    DOMNode* node = list->item( 0 ) ;
    DOMElement* elementConfig( NULL );
    if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
        elementConfig = dynamic_cast< DOMElement* >( node ) ;

    if ( NULL == elementConfig )
        throw( runtime_error( "empty XML document" ) );

    // retrieving attributs
    get_attr_value( elementConfig, "connect_port", &m_Data.m_nPort );
    get_attr_value( elementConfig, "server_addr", &m_Data.m_strHost );

    InfoLog( erOK, "Agent Client configuration:" ) << m_Data;

    return erOK;
}

ERRORCODE CAgentClient::Write( xercesc::DOMNode* _element )
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

        stringstream ssMsg;
        ssMsg
        << "connecting to " << m_Data.m_strHost
        << "on port " << m_Data.m_nPort << "...";

        client.Connect( m_Data.m_nPort, m_Data.m_strHost );
        DebugLog( erOK, "connected!" );

        //             { // a transfer test
        //                 string sTest( "Test String!" );
        //                 BYTEVector_t buf;
        //                 copy( sTest.begin(), sTest.end(), back_inserter( buf ) );
        //                 m_pThis->LogThread( "Sending: " + string( ( char* ) & buf[ 0 ] ) );
        //                 client.GetSocket() << buf;
        //             }

        // Spwan PortForwarder
        CPacketForwarder pf( client.GetSocket(), NEW_PORT );
        pf.Start();
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}
