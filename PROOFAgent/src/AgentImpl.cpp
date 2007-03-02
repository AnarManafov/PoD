/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
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

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
XERCES_CPP_NAMESPACE_USE;
using namespace PROOFAgent;

//------------------------- Agent SERVER ------------------------------------------------------------
struct SServerThread
{
    SServerThread( CAgentServer * _pThis ) : m_pThis( _pThis )
    {}
    void operator() ()
    {
        m_pThis->LogThread( "Starting main thread..." );
        smart_socket SocketListener = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( SocketListener < 0 )
        {
            m_pThis->LogThread( "Soket error..." ); // TODO: perror( "socket" );
            return ;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons( m_pThis->m_Data.m_nPort );
        addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
        if ( bind( SocketListener, ( struct sockaddr * ) & addr, sizeof( addr ) ) < 0 )
        {
            m_pThis->LogThread( "Soket bind error..." ); // TODO: perror( "bind" );
            return ;
        }

        ::listen( SocketListener, 1 );
        std::stringstream ssMsg;
        ssMsg << "Listenening on port #" << m_pThis->m_Data.m_nPort << " ...";
        m_pThis->LogThread( ssMsg.str() );
        while ( true )
        {
            smart_socket sock( ::accept( SocketListener, NULL, NULL ) );
            if ( sock < 0 )
            {
                m_pThis->LogThread( "Soket accept error..." ); // TODO: perror("accept");
                return ;
            }

        }
    }
private:
    CAgentServer * m_pThis;
};

ERRORCODE CAgentServer::Init( DOMNode* _element )
{
    return Read( _element );
}

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

ERRORCODE CAgentServer::Start()
{
    boost::thread thrd_server( SServerThread( this ) );
    thrd_server.join();

    return erOK;
}

//------------------------- Agent CLIENT ------------------------------------------------------------
ERRORCODE CAgentClient::Init( xercesc::DOMNode* _element )
{
    return erNotImpl;
}

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

ERRORCODE CAgentClient::Start()
{
    return MiscCommon::erNotImpl;
}
