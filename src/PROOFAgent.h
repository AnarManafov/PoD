/************************************************************************/
/**
 * @file PROOFAgent.h
 * @brief Header of the general PROOFAgent manager
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENT_H
#define PROOFAGENT_H

// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// PROOFAgent
#include "Agent.h"

/**
 *
 * @brief A general name space for PROOFAgent application
 *
 */
namespace PROOFAgent
{
//    /**
//     *
//     * @brief A custom DOM error handler. Required by DOMBuilder.
//     * @exception std::runtime_error - thrown in any case of error from the parser.
//     * @note We don't check severity (_domError.getSeverity) of the error.
//     * @note We abort parsing in any way by throwing the exception.
//     *
//     */
//    class CDOMErrorHandler: public xercesc::DOMErrorHandler
//    {
//        public:
//            bool handleError( const xercesc::DOMError &_domError ) throw( std::exception )
//            {
//                MiscCommon::XMLHelper::smart_XMLCh msg( _domError.getMessage() );
//                xercesc::DOMLocator *locator( _domError.getLocation() );
//                std::ostringstream ss;
//                ss
//                << "XML DOM Error "
//                << "in [" << MiscCommon::XMLHelper::smart_XMLCh( locator->getURI() ).ToString() << "] "
//                << "at position " << locator->getLineNumber()
//                << ":"  << locator->getColumnNumber()
//                << " \"" << msg.ToString() << "\"";
//                throw std::runtime_error( ss.str() );
//            }
//    };
/**
 *
 * @brief Agent's data structure.
 *
 */
typedef struct SAgentData
{
    SAgentData() :
            m_isServerMode(true),
            m_bLogFileOverwrite( false ),
            m_nTimeout( 0 ),
            m_sWorkDir( "/tmp/" )
    {}
    bool m_isServerMode; 			//!< Specify in which operation mode PA is started, Server of Client (default: true)
    std::string m_sLogFileDir;      //!< The log filename.
    bool m_bLogFileOverwrite;       //!< Overwrite log file each session.
    EAgentMode_t m_AgentMode;       //!< A mode of PROOFAgent, defined by ::EAgentMode_t.
    /**
     *
     * @brief It is a number of seconds, represents the time PROOFAgent instance is allowed to work.
     * @brief An internal timeout guard of the PROOFAgent will not allow PROOF agent to work longer, than
     * @brief it is instructed by this value and PROOFAgent will be forced to be killed. Default is 0 - no timeout.
     *
     */
    size_t m_nTimeout;
    std::string m_sLastExecCmd;     //!< PROOFAgent will execute this command at the end of the session.
    std::string m_sPROOFCfg;        //!< A location of the proof configuration file.
    std::string m_sWorkDir;         //!< Working folder. (default: /tmp/)
}
SAgentData_t;
/**
 *
 * @brief The PROOFAgent manager
 *
 */
class CPROOFAgent:
            public MiscCommon::CLogImp<CPROOFAgent> //,
            //MiscCommon::IXMLPersistImpl<CPROOFAgent>
{
    friend class boost::serialization::access;
public:
    CPROOFAgent();
    virtual ~CPROOFAgent();

    REGISTER_LOG_MODULE( "PROOFAgent" )
    // DECLARE_XMLPERSIST_IMPL( CPROOFAgent )

public:
    // void ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML = false ) throw( std::exception );
    void Start() throw( std::exception );

private:
//            BEGIN_READ_XML_CFG( CPROOFAgent )
//            READ_NODE_VALUE( "work_dir", m_Data.m_sWorkDir )
//            READ_NODE_VALUE( "logfile_dir", m_Data.m_sLogFileDir )
//            READ_NODE_VALUE( "logfile_overwrite", m_Data.m_bLogFileOverwrite )
//            READ_NODE_VALUE( "timeout", m_Data.m_nTimeout )
//            READ_NODE_VALUE( "last_execute_cmd", m_Data.m_sLastExecCmd )
//            READ_NODE_VALUE( "proof_cfg_path", m_Data.m_sPROOFCfg )
//            END_READ_XML_CFG
//
//            BEGIN_WRITE_XML_CFG( CPROOFAgent )
//            END_WRITE_XML_CFG
    template<class Archive>
    void save( Archive & _ar, const unsigned int /*_version*/ ) const
    {
        _ar
        & BOOST_SERIALIZATION_NVP( m_Data.m_isServerMode )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sWorkDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLogFileDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_bLogFileOverwrite )
        & BOOST_SERIALIZATION_NVP( m_Data.m_nTimeout )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLastExecCmd )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sPROOFCfg );
    }
    template<class Archive>
    void load( Archive & _ar, const unsigned int /*_version*/ )
    {
        _ar
        & BOOST_SERIALIZATION_NVP( m_Data.m_isServerMode )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sWorkDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLogFileDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_bLogFileOverwrite )
        & BOOST_SERIALIZATION_NVP( m_Data.m_nTimeout )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLastExecCmd )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sPROOFCfg );

        // Correcting configuration values
        // resolving user's home dir from (~/ or $HOME, if present)
        MiscCommon::smart_path( &m_Data.m_sWorkDir );
        // We need to be sure that there is "/" always at the end of the path
        MiscCommon::smart_append<std::string>( &m_Data.m_sWorkDir, '/' );

        MiscCommon::smart_path( &m_Data.m_sLogFileDir );
        MiscCommon::smart_append<std::string>( &m_Data.m_sLogFileDir, '/' );

        MiscCommon::smart_path( &m_Data.m_sPROOFCfg );

        m_Data.m_AgentMode = ( m_Data.m_isServerMode ) ? Server : Client;

        // Initializing log engine
        // log file name: proofagent.<instance_name>.pid
        std::stringstream logfile_name;
        logfile_name
        << m_Data.m_sLogFileDir
        << "proofagent."
        << (( m_Data.m_isServerMode ) ? server : client)
        << ".log";

        MiscCommon::CLogSinglton::Instance().Init( logfile_name.str(), m_Data.m_bLogFileOverwrite );
        InfoLog( erOK, PACKAGE + string( " v." ) + VERSION );

        // Timeout Guard
        if ( 0 != m_Data.m_nTimeout )
            CTimeoutGuard::Instance().Init( getpid(), m_Data.m_nTimeout );

        // Spawning new Agent in requested mode
        m_Agent.SetMode( m_Data.m_AgentMode );
        m_Agent.Init( instance );
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    void ExecuteLastCmd();
    // void _ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML ) throw( std::exception );

private:
    SAgentData_t m_Data;
    CAgent m_Agent;
    std::string m_cfgFileName;
};
BOOST_CLASS_VERSION( CPROOFAgent, 1 )
};
#endif

