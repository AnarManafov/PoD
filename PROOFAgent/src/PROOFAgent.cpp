/************************************************************************/
/**
 * @file PROOFAgent.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// STD
#include <stdexcept>
#include <iostream>
#include <string>

// XML parser
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

// PROOFAgent
#include "XMLHelper.h"
#include "PROOFAgent.h"
#include "MiscUtils.h"

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
using namespace PROOFAgent;
XERCES_CPP_NAMESPACE_USE;


ERRORCODE CPROOFAgent::Init( const string &_xmlFileName )
{
    ERRORCODE er = ReadCfg( _xmlFileName );
    if ( erOK != er )
        return er;

    return m_Agent.Start();
}

ERRORCODE CPROOFAgent::ReadCfg( const std::string &_xmlFileName )
{
    string xmlFileName;
    char *p = getenv( "PROOFAGENT_LOCATION" );
    if ( p )
    {
        xmlFileName = string( p ) + "/etc/" + _xmlFileName;
    }
    else
    {
        xmlFileName = _xmlFileName;
    }

    // Initializing XML parser - Xerces-C++
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch ( const XMLException & toCatch )
    {
        string errMsg( "Error occurred while initializing XML parser: " );
        errMsg += smart_XMLCh( toCatch.getMessage() ).ToString();
        FaultLog( erXMLInit, errMsg );
        return erXMLInit;
    }

    auto_ptr<XercesDOMParser> parser( new XercesDOMParser() );
    parser->setValidationScheme( XercesDOMParser::Val_Always ); // optional.
    parser->setDoNamespaces( true ); // optional
    auto_ptr<ErrorHandler> errHandler( dynamic_cast<ErrorHandler*>( new HandlerBase() ) );
    parser->setErrorHandler( errHandler.get() );

    try
    {
        // Parsing an xml
        parser->parse( xmlFileName.c_str() );

        // Creating DOM document object
        DOMDocument* xmlDoc = parser->getDocument();

        // getting <gaw_config> Element
        smart_XMLCh ElementName( "proofagent_config" );
        DOMNodeList *list = xmlDoc->getElementsByTagName( ElementName );

        DOMNode* node = list->item( 0 ) ;
        DOMElement* elementConfig( NULL );
        if ( xercesc::DOMNode::ELEMENT_NODE == node->getNodeType() )
            elementConfig = dynamic_cast< xercesc::DOMElement* >( node ) ;

        //   DOMElement* elementConfig = xmlDoc->getDocumentElement();

        if ( NULL == elementConfig )
            throw( runtime_error( "empty XML document" ) );

        // Reading Agent's configuration
        {
            smart_XMLCh ElementName( "agent" );
            DOMNodeList *list = xmlDoc->getElementsByTagName( ElementName );
            DOMNode* node = list->item( 0 );
            Read( node );
            // Initializing log engine
            CLogSinglton::Instance().Init( m_Data.m_sLogFileName, m_Data.m_bLogFileOverwrite );
            InfoLog( erOK, PACKAGE_STRING );
        }
        // retrieving "version" attribut
        {
            smart_XMLCh ATTR_VERSION( "version" );
            smart_XMLCh xmlTmpStr( elementConfig->getAttribute( ATTR_VERSION ) );
            string config_version( xmlTmpStr.ToString() );
            DebugLog( erOK, "PROOFAgent configuration version: " + config_version );
        }

        {
            // Spawning new Agent in requasted mode
            m_Agent.SetMode( m_Data.m_AgentMode );
            m_Agent.Init( node );
        }

    }
    catch ( const XMLException & toCatch )
    {
        string errMsg( "Error occurred while reading XML configuration file: " );
        errMsg += smart_XMLCh( toCatch.getMessage() ).ToString();
        FaultLog( erXMLInit, errMsg );
        return erXMLReadConfig;
    }
    catch ( const DOMException & toCatch )
    {
        string errMsg( "Error occurred while reading XML configuration file: " );
        errMsg += smart_XMLCh( toCatch.msg ).ToString();
        FaultLog( erXMLInit, errMsg );
        return erXMLReadConfig;
    }
    catch ( const exception & toCatch )
    {
        string errMsg( "Error occurred while reading XML configuration file: " );
        errMsg += smart_XMLCh( toCatch.what() ).ToString();
        FaultLog( erXMLInit, errMsg );
        return erXMLReadConfig;
    }
    catch ( ... )
    {
        string errMsg( "Unexpected Exception occured while reading configuration file: " + xmlFileName );
        FaultLog( erXMLInit, errMsg );
        return erXMLReadConfig;
    }

    if ( NULL != parser.get() )
        delete parser.release();
    if ( NULL != errHandler.get() )
        delete errHandler.release();

    XMLPlatformUtils::Terminate();
    return erOK;
}

ERRORCODE CPROOFAgent::Read( xercesc::DOMNode* _element )
{
    // TODO: Use a "try" block to catch XML exceptions
    if ( NULL == _element )
        return erXMLNullNode;

    // getting <config> Element
    smart_XMLCh ElementName( "config" );
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
    get_attr_value( elementConfig, "logfile", &m_Data.m_sLogFileName );
    get_attr_value( elementConfig, "logfile_overwrite", &m_Data.m_bLogFileOverwrite );
    string sValTmp;
    get_attr_value( elementConfig, "agent_mode", &sValTmp );
    MiscCommon::to_lower( sValTmp );
    m_Data.m_AgentMode = ( sValTmp.find( "server" ) != sValTmp.npos ) ? Server : Client;

    return erOK;
}

ERRORCODE CPROOFAgent::Write( xercesc::DOMNode* _element )
{
    return erNotImpl;
}

