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

// PROOFAgent
#include "Options.h"
#include "Agent.h"

//=============================================================================
/**
 *
 * @brief A general name space for PROOFAgent application
 *
 */
namespace PROOFAgent
{
//=============================================================================
    /**
     *
     * @brief The PROOFAgent manager
     *
     */
    class CPROOFAgent:
        public MiscCommon::CLogImp<CPROOFAgent>
    {
        public:
            CPROOFAgent();
            virtual ~CPROOFAgent();

            REGISTER_LOG_MODULE( "PROOFAgent" )

        public:
            void setConfiguration( const SOptions_t &_data );
            void Start() throw( std::exception );

        private:
            void initLogEngine();

        private:
            PoD::SCommonOptions_t m_Data;
            EAgentMode_t m_Mode;
            CAgent m_Agent;
    };

};

#endif

