/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
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
    typedef enum{ Server, Client }EAgentMode_t;

    /** @brief WMSUI data structur. */
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
                public glite_api_wrapper::CLogImp<CPROOFAgent>,
                IXMLPersist
    {
        public:
            std::string _GetModuleName() const
            {
                return "PROOFAgent";
            }

            ERRORCODE Init( const std::string &_xmlFileName );
            ERRORCODE Read( xercesc::DOMNode* _element );
            ERRORCODE Write( xercesc::DOMNode* _element );

        private:
            SAgentData_t m_Data;
    };

};
#endif

