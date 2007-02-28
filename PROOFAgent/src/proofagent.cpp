/************************************************************************/
/**
 * @file proofagent.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// STD
#include <iostream> 
// PROOFAgent
#include "agent.h"
#include "XMLHelper.h"
#include "LogImp.h"

using namespace std;
using namespace proofagent;
using namespace glite_api_wrapper;
XERCES_CPP_NAMESPACE_USE;


/** @brief WMSUI data structur. */
typedef struct SAgentData
{
    SAgentData() : m_bLogFileOverwrite( false )
    {}
    std::string m_sLogFileName;     //!< Log filename
    bool m_bLogFileOverwrite;       //!< Overwrite log file each session
}
SAgentData_t;

class CPROOFAgent:
            public CLogImp<CPROOFAgent>,
            IXMLPersist
{
    public:
        ERRORCODE Init( const string &_xmlFileName )
        {
            string xmlFileName;
            char *p = getenv( "PROOFAGENT_LOCATION" );
            if ( p )
            {
                xmlFileName = string( p ) + "/etc/" + _xmlFileName;
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
                    InfoLog( erOK, g_cszMsgStartCore );
                }

//                 // retrieving "version" attribut
//                 {
//                     smart_XMLCh ATTR_VERSION( "version" );
//                     smart_XMLCh xmlTmpStr( elementConfig->getAttribute( ATTR_VERSION ) );
//                     string config_version( xmlTmpStr.ToString() );
//                     DebugLog( erOK, "GAW configuration version: " + config_version );
//                 }

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

        ERRORCODE Read( xercesc::DOMNode* _element )
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
            string sValTmp;
            get_attr_value( elementConfig, "logfile_overwrite", &sValTmp );
            m_Data.m_bLogFileOverwrite = !( sValTmp.empty() || ( "0" == sValTmp ) );

            return erOK;
        }
        ERRORCODE WriteConfig( const std::string &_xmlFileName )
        {
            return erNotImpl;
        }

    private:
        SAgentData_t m_Data;
};


int main( int argc, char *argv[] )
{
    CAgent<CAgentServer> server;
    cout << server.Init() << endl;

    return erOK;
}
