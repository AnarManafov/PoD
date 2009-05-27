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
#include "TimeoutGuard.h"

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
            //void loadCfg( const std::string &_fileName );
        	void setConfiguration( const SOptions_t *_data );
            void Start() throw( std::exception );

        private:

//           template<class Archive>
//            void load( Archive & _ar, const unsigned int /*_version*/ )
//            {
//                _ar
//                & BOOST_SERIALIZATION_NVP( m_Data.m_isServerMode )
//                & BOOST_SERIALIZATION_NVP( m_Data.m_sWorkDir );
//                initLogEngine();
//
//                _ar
//                & BOOST_SERIALIZATION_NVP( m_Data.m_sLogFileDir )
//                & BOOST_SERIALIZATION_NVP( m_Data.m_bLogFileOverwrite )
//                & BOOST_SERIALIZATION_NVP( m_Data.m_nTimeout )
//                & BOOST_SERIALIZATION_NVP( m_Data.m_sLastExecCmd )
//                & BOOST_SERIALIZATION_NVP( m_Data.m_sPROOFCfg );
//
//                m_Data.m_AgentMode = ( m_Data.m_isServerMode ) ? Server : Client;
//                // Spawning new Agent in requested mode
//                m_Agent.SetMode( m_Data.m_AgentMode );
//                _ar & BOOST_SERIALIZATION_NVP( m_Agent );
//            }
//            BOOST_SERIALIZATION_SPLIT_MEMBER()

            void ExecuteLastCmd();
            void initLogEngine();

        private:
            SAgentData_t m_Data;
            CAgent m_Agent;
            std::string m_cfgFileName;
    };

};

#endif

