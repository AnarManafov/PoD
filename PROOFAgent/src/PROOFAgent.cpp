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
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

// PROOFAgent
#include "XMLHelper.h"
#include "PROOFAgent.h"
#include "MiscUtils.h"
#include "TimeoutGuard.h"


using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
using namespace PROOFAgent;
XERCES_CPP_NAMESPACE_USE;


ERRORCODE CPROOFAgent::Start()
{
    return m_Agent.Start();
}

ERRORCODE CPROOFAgent::ReadCfg( const std::string &_xmlFileName, const std::string &_Instance )
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

        // getting <proofagent_config> Element
        DOMNode *node = GetSingleNodeByName_Ex( xmlDoc, "proofagent_config" );

        DOMElement* elementConfig( NULL );
        if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
            elementConfig = dynamic_cast< xercesc::DOMElement* >( node ) ;

        if ( !elementConfig )
            throw( runtime_error( "empty XML document" ) );

        // Reading Agent's configuration
        {
            // <instances>
            DOMNode *instances_node = GetSingleNodeByName_Ex( node, "instances" );

            // <instance>
            DOMNode *instance = GetSingleNodeByName_Ex( instances_node, _Instance );

            // reading configuration of a given instance
            Read( instance );

            // Initializing log engine
            // log file name: proofagent.<instance_name>.pid
            stringstream logfile_name;
            logfile_name
            << m_Data.m_sLogFileDir
            << "proofagent."
            << _Instance
            << ".log";

            CLogSinglton::Instance().Init( logfile_name.str(), m_Data.m_bLogFileOverwrite );
            InfoLog( erOK, PACKAGE + string(" v.") + VERSION );

            // Timeout Guard
            if ( 0 != m_Data.m_nTimeout )
                CTimeoutGuard::Instance().Init( getpid(), m_Data.m_nTimeout );
        }

        // retrieving "version" attribute
        {
            smart_XMLCh ATTR_VERSION( "version" );
            smart_XMLCh xmlTmpStr( elementConfig->getAttribute( ATTR_VERSION ) );
            string config_version( xmlTmpStr.ToString() );
            DebugLog( erOK, "PROOFAgent configuration version: " + config_version );
        }

        {
            // Spawning new Agent in requested mode
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
        string errMsg( "Unexpected Exception occurred while reading configuration file: " + xmlFileName );
        FaultLog( erXMLInit, errMsg );
        return erXMLReadConfig;
    }

    // TODO: Avoid of MemLeaks
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
    DOMNode *node = GetSingleNodeByName_Ex( _element, "config" );

    DOMElement* elementConfig( NULL );
    if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
        elementConfig = dynamic_cast< DOMElement* >( node ) ;
    if ( NULL == elementConfig )
        throw( runtime_error( "empty XML document" ) );

    // retrieving attributes
    get_attr_value( elementConfig, "logfile_dir", &m_Data.m_sLogFileDir );
    // We need to be sure that there is "/" always at the end of the path
    smart_append<string>( &m_Data.m_sLogFileDir, '/' );
    get_attr_value( elementConfig, "logfile_overwrite", &m_Data.m_bLogFileOverwrite );
    string sValTmp;
    get_attr_value( elementConfig, "agent_mode", &sValTmp );
    MiscCommon::to_lower( sValTmp );
    m_Data.m_AgentMode = ( sValTmp.find( "server" ) != sValTmp.npos ) ? Server : Client;
    get_attr_value( elementConfig, "timeout", &m_Data.m_nTimeout );

    return erOK;
}

ERRORCODE CPROOFAgent::Write( xercesc::DOMNode* _element )
{
    return erNotImpl;
}

