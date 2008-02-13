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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENT_H
#define PROOFAGENT_H

// PROOFAgent
#include "Agent.h"

/**
 * @brief A general name space for PROOFAgent application
 **/
namespace PROOFAgent
{
    /**
     *
     * @brief A custom DOM error handler. Requared by DOMBuilder.
     * @exception std::runtime_error - thrown in any case of error from the parser.
     * @note We don't check severity (_domError.getSeverity) of the error.
     * @note We abort parsing in any way by throwing the exception.
     *
     */
    class CDOMErrorHandler: public xercesc::DOMErrorHandler
    {
        public:
            bool handleError (const xercesc::DOMError &_domError) throw(std::exception)
            {
                MiscCommon::XMLHelper::smart_XMLCh msg( _domError.getMessage() );
                xercesc::DOMLocator *locator( _domError.getLocation() );
                std::ostringstream ss;
                ss
                << "XML DOM Error "
                << "in [" << MiscCommon::XMLHelper::smart_XMLCh( locator->getURI() ).ToString() << "] "
                << "at position " << locator->getLineNumber()
                << ":"  << locator->getColumnNumber()
                << " \"" << msg.ToString() << "\"";
                throw std::runtime_error( ss.str() );
            }
    };
    /**
     * 
     * @brief Agent data structure.
     * 
     */
    typedef struct SAgentData
    {
        SAgentData() :
                m_bLogFileOverwrite( false ),
                m_nTimeout( 0 ),
                m_sWorkDir("/tmp/")
        {}
        std::string m_sLogFileDir;          //!< Log filename
        bool m_bLogFileOverwrite;          //!< Overwrite log file each session
        EAgentMode_t m_AgentMode;
        std::string m_sAgentMode;
        size_t m_nTimeout;
        std::string m_sLastExecCmd;
        std::string m_sPROOFCfg;
        std::string m_sWorkDir;         //!< Working folder. (default: /tmp/)
    }
    SAgentData_t;

    class CPROOFAgent:
                public MiscCommon::CLogImp<CPROOFAgent>,
                MiscCommon::IXMLPersistImpl<CPROOFAgent>
    {
        public:
            CPROOFAgent()
            {}
            virtual ~CPROOFAgent()
            {
                ExecuteLastCmd();
            }
            REGISTER_LOG_MODULE( "PROOFAgent" )
            DECLARE_XMLPERSIST_IMPL(CPROOFAgent)

        public:
            void ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML = false ) throw(std::exception);
            void Start() throw(std::exception);

        private:
            BEGIN_READ_XML_CFG(CPROOFAgent)            
            READ_NODE_VALUE( "work_dir", m_Data.m_sWorkDir )
            READ_NODE_VALUE( "logfile_dir", m_Data.m_sLogFileDir )
            READ_NODE_VALUE( "logfile_overwrite", m_Data.m_bLogFileOverwrite )            
            READ_NODE_VALUE( "timeout", m_Data.m_nTimeout )
            READ_NODE_VALUE( "last_execute_cmd", m_Data.m_sLastExecCmd )
            READ_NODE_VALUE( "proof_cfg_path", m_Data.m_sPROOFCfg )
            END_READ_XML_CFG

            BEGIN_WRITE_XML_CFG(CPROOFAgent)
            END_WRITE_XML_CFG

            void ExecuteLastCmd();
            void _ReadCfg( const std::string &_xmlFileName, const std::string &_Instance, bool _bValidateXML ) throw(std::exception);

        private:
            SAgentData_t m_Data;
            CAgent m_Agent;
    };

};
#endif

