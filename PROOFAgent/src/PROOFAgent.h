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

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENT_H
#define PROOFAGENT_H

// PROOFAgent
#include "Agent.h"
#include "IXMLPersist.h"
#include "LogImp.h"

/**
 * @brief A general name space for PROOFAgent application
 **/
namespace PROOFAgent
{
    /**
     * @brief Agent data structure.
     **/
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
            void ReadCfg( const std::string &_xmlFileName, const std::string &_Instance ) throw(std::exception);
            void Start() throw(std::exception);

        private:
            BEGIN_READ_XML_CFG(CPROOFAgent)
            READ_ELEMENT( "work_dir", m_Data.m_sWorkDir )
            READ_ELEMENT( "logfile_dir", m_Data.m_sLogFileDir )
            READ_ELEMENT( "logfile_overwrite", m_Data.m_bLogFileOverwrite )
            READ_ELEMENT( "agent_mode", m_Data.m_sAgentMode )
            READ_ELEMENT( "timeout", m_Data.m_nTimeout )
            READ_ELEMENT( "last_execute_cmd", m_Data.m_sLastExecCmd )
            READ_ELEMENT( "proof_cfg_path", m_Data.m_sPROOFCfg )
            END_READ_XML_CFG

            BEGIN_WRITE_XML_CFG(CPROOFAgent)
            END_WRITE_XML_CFG

            void ExecuteLastCmd();

        private:
            SAgentData_t m_Data;
            CAgent m_Agent;
    };

};
#endif

