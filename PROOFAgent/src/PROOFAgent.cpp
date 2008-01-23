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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
// STD
#include <stdexcept>
// PROOFAgent
#include "PROOFAgent.h"
// MiscCommon
#include "MiscUtils.h"
#include "TimeoutGuard.h"
#include "SysHelper.h"
#include "FindCfgFile.h"

#define MAX_PATH 1024

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::XMLHelper;
using namespace PROOFAgent;
XERCES_CPP_NAMESPACE_USE;


void CPROOFAgent::Start() throw(exception)
{
    m_Agent.Start( m_Data.m_sPROOFCfg );
}

void CPROOFAgent::ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML ) throw(exception)
{
    // Strategy if looking for Cfg file:
    // 1 - current working directory
    // 2 - $HOME/
    // 3 - $PROOFAGENT_LOCATION/etc/
    // 4 - /etc/
    string cur_dir;
    CHARVector_t buf( MAX_PATH );
    if ( ::getcwd( &buf[0], MAX_PATH ) )
    {
        string path( &buf[0] );
        smart_append( &path, '/' );
        cur_dir += _xmlFileName;
    }

    CFindCfgFile<string> cfg_file;
    cfg_file.SetOrder
    (cur_dir)
    ("$HOME/" + _xmlFileName)
    ("$PROOFAGENT_LOCATION/etc/" + _xmlFileName)
    ("/etc/" + _xmlFileName);

    string xmlFileName;
    cfg_file.GetCfg( &xmlFileName );

    smart_path( &xmlFileName );

    _ReadCfg( xmlFileName, _Instance, _bValidateXML );
}

void CPROOFAgent::_ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML ) throw(exception)
{
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

    // Getting DOMImplementation
    smart_XMLCh features("LS");
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation( features );
    if ( !impl )
        throw runtime_error( "Can't get DOMImplementation object." );

    try
    {
        // Getting DOMBuilder
        boost::shared_ptr<DOMBuilder> parser(
            (dynamic_cast<DOMImplementationLS*>(impl))->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, NULL ),
            boost::mem_fn(&DOMBuilder::release)
        );

        if ( parser->canSetFeature(XMLUni::fgDOMNamespaces, true) )
            parser->setFeature( XMLUni::fgDOMNamespaces, true );
        if ( parser->canSetFeature(XMLUni::fgXercesSchema, _bValidateXML) )
            parser->setFeature( XMLUni::fgXercesSchema, _bValidateXML );
        if ( parser->canSetFeature(XMLUni::fgXercesSchemaFullChecking, _bValidateXML) )
            parser->setFeature( XMLUni::fgXercesSchemaFullChecking, _bValidateXML );
        if ( parser->canSetFeature(XMLUni::fgDOMValidation, _bValidateXML) )
            parser->setFeature( XMLUni::fgDOMValidation, _bValidateXML );
        // enable datatype normalization - default is off
        if ( parser->canSetFeature(XMLUni::fgDOMDatatypeNormalization, true) )
            parser->setFeature( XMLUni::fgDOMDatatypeNormalization, true );

        // Setting up the custom error handler
        auto_ptr<CDOMErrorHandler> errHandler( new CDOMErrorHandler() );
        parser->setErrorHandler( errHandler.get() );

        // Creating DOM document object from an XML file
        DOMDocument* xmlDoc(
            parser->parseURI( _xmlFileName.c_str() )
        );

        // getting <proofagent_config> Element
        DOMNode *node = GetSingleNodeByName_Ex( xmlDoc, "proofagent_config" );

        DOMElement* elementConfig( NULL );
        if ( DOMNode::ELEMENT_NODE == node->getNodeType() )
            elementConfig = dynamic_cast< xercesc::DOMElement* >( node ) ;

        if ( !elementConfig )
            throw( runtime_error( "empty XML document." ) );

        // Reading Agent's configuration

        // <instances>
        DOMNode *instances_node = GetSingleNodeByName_Ex( node, "instances" );

        // Getting all "instance" nodes
        const DOMNodeList *list = GetNodesByName(instances_node, "instance" );
        if ( !list )
            throw( runtime_error( "can't find \"instance\" XML node." ) );
        DOMNode *instance = NULL;
        // Loopig through "instance" nodes
        for (XMLSize_t i = 0; i < list->getLength(); ++i)
        {
            DOMNode *inst( list->item(i) );
            if ( !inst )
                continue;
            string sNodeName;
            get_attr_value( dynamic_cast<DOMElement*>(inst), "name", &sNodeName );
            if (_Instance == sNodeName )
                instance = inst;
        }
        if ( !instance )
            throw runtime_error( "can't find \"instance\" XML node: " + _Instance );

        // reading configuration of a given instance
        Read( instance );
        // Correcting convfiguration values
        // resolving user's home dir from (~/ or $HOME, if present)
        smart_path( &m_Data.m_sWorkDir );
        // We need to be sure that there is "/" always at the end of the path
        smart_append<string>( &m_Data.m_sWorkDir, '/' );

        smart_path( &m_Data.m_sLogFileDir );
        smart_append<string>( &m_Data.m_sLogFileDir, '/' );

        // Reading type of the instance
        get_attr_value( dynamic_cast<DOMElement*>(instance), "type", &m_Data.m_sAgentMode );
        m_Data.m_AgentMode = ( m_Data.m_sAgentMode.find( "server" ) != m_Data.m_sAgentMode.npos ) ? Server : Client;

        smart_path( &m_Data.m_sPROOFCfg );

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
