/************************************************************************/
/**
 * @file PROOFAgent.cpp
 * @brief Implementation of the general PROOFAgent manager
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
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
#include "PROOFAgent.h"
#include "MiscUtils.h"
#include "TimeoutGuard.h"
#include "SysHelper.h"


using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
using namespace PROOFAgent;
XERCES_CPP_NAMESPACE_USE;


void CPROOFAgent::Start() throw(exception)
{
    m_Agent.Start( m_Data.m_sPROOFCfg );
}

void CPROOFAgent::ReadCfg( const std::string &_xmlFileName, const std::string &_Instance ) throw(exception)
{
    string xmlFileName;
    char *p = getenv( "PROOFAGENT_LOCATION" );
    xmlFileName = p ? (string( p ) + "/etc/" + _xmlFileName) : _xmlFileName;

    // Initializing XML parser - Xerces-C++
    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch ( const XMLException & toCatch )
    {
        string errMsg( "Error occurred while initializing XML parser: " );
        errMsg += smart_XMLCh( toCatch.getMessage() ).ToString();
        throw runtime_error( errMsg );
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

        // <instances>
        DOMNode *instances_node = GetSingleNodeByName_Ex( node, "instances" );

        // <instance>
        DOMNode *instance = GetSingleNodeByName_Ex( instances_node, _Instance );

        // reading configuration of a given instance
        Read( instance );
        // Correcting convfiguration values
        // resolving user's home dir from (~/ or $HOME, if present)
        smart_homedir_append( &m_Data.m_sWorkDir );
        // We need to be sure that there is "/" always at the end of the path
        smart_append<string>( &m_Data.m_sWorkDir, '/' );

        smart_homedir_append( &m_Data.m_sLogFileDir );
        smart_append<string>( &m_Data.m_sLogFileDir, '/' );

        MiscCommon::to_lower( m_Data.m_sAgentMode );
        m_Data.m_AgentMode = ( m_Data.m_sAgentMode.find( "server" ) != m_Data.m_sAgentMode.npos ) ? Server : Client;

        smart_homedir_append( &m_Data.m_sPROOFCfg );

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

        // retrieving "version" attribute
        smart_XMLCh ATTR_VERSION( "version" );
        smart_XMLCh xmlTmpStr( elementConfig->getAttribute( ATTR_VERSION ) );
        string config_version( xmlTmpStr.ToString() );
        DebugLog( erOK, "PROOFAgent configuration version: " + config_version );

        // Spawning new Agent in requested mode
        m_Agent.SetMode( m_Data.m_AgentMode );
        m_Agent.Init( instance );
    }
    catch ( const XMLException & toCatch )
    {
        string errMsg( "Error occurred while reading XML configuration file: " );
        errMsg += smart_XMLCh( toCatch.getMessage() ).ToString();
        throw runtime_error( errMsg );
    }
    catch ( const DOMException & toCatch )
    {
        string errMsg( "Error occurred while reading XML configuration file: " );
        errMsg += smart_XMLCh( toCatch.msg ).ToString();
        throw runtime_error( errMsg );
    }

    // TODO: Avoid of MemLeaks
    if ( NULL != parser.get() )
        delete parser.release();
    if ( NULL != errHandler.get() )
        delete errHandler.release();

    XMLPlatformUtils::Terminate();
}

void CPROOFAgent::ExecuteLastCmd()
{
    if ( !m_Data.m_sLastExecCmd.empty() )
    {
        InfoLog( erOK, "executing last command: " + m_Data.m_sLastExecCmd );
        if ( -1 == ::system( m_Data.m_sLastExecCmd.c_str() ) )
            FaultLog( erError, "Can't execute last command: " + m_Data.m_sLastExecCmd );
    }
}
