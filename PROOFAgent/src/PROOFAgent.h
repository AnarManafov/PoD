/************************************************************************/
/**
 * @file PROOFAgent.h
 * @brief Header of the general PROOFAgent manager
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
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
        size_t m_nTimeout;
        std::string m_sLastExecCmd;
        std::string m_sPROOFCfg;
        std::string m_sWorkDir;         //!< Working folder. (default: /tmp/)
    }
    SAgentData_t;

    class CPROOFAgent:
                public MiscCommon::CLogImp<CPROOFAgent>,
                MiscCommon::IXMLPersist
    {
        public:
            CPROOFAgent()
            {}
            virtual ~CPROOFAgent()
            {
                ExecuteLastCmd();
            }
            REGISTER_LOG_MODULE( PROOFAgent )

        public:
            MiscCommon::ERRORCODE ReadCfg( const std::string &_xmlFileName, const std::string &_Instance );
            MiscCommon::ERRORCODE Start();

        private:
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );
            void ExecuteLastCmd();

        private:
            SAgentData_t m_Data;
            CAgent m_Agent;
    };

};
#endif

