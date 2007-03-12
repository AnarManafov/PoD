/************************************************************************/
/**
 * @file PROOFAgent.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENT_H
#define PROOFAGENT_H

// PROOFAgent
#include "Agent.h"
#include "IXMLPersist.h"
#include "LogImp.h"

namespace PROOFAgent
{
    /** @brief Agent data structur. */
    typedef struct SAgentData
    {
        SAgentData() : m_bLogFileOverwrite( false )
        {}
        std::string m_sLogFileName;     //!< Log filename
        bool m_bLogFileOverwrite;       //!< Overwrite log file each session
        EAgentMode_t m_AgentMode;
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
            {}
            REGISTER_LOG_MODULE( PROOFAgent )

        public:
            MiscCommon::ERRORCODE Init( const std::string &_xmlFileName );

        private:
            MiscCommon::ERRORCODE ReadCfg( const std::string &_xmlFileName );
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );

        private:
            SAgentData_t m_Data;
            CAgent m_Agent;
    };

};
#endif

