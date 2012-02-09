/************************************************************************/
/**
 * @file Agent.h
 * @brief Implementation of CAgent
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENT_H
#define AGENT_H

// STD
#include <memory>
// PROOFAgent
#include "AgentServer.h"
#include "AgentClient.h"

namespace PROOFAgent
{
    typedef boost::shared_ptr<CAgentBase> pAgentBase_t;
    /**
     *
     * @brief A simple object factory class.
     * @brief It produces Agents objects according to the given mode.
     *
     */
    class CAgent
    {
        public:
            CAgent( EAgentMode_t _Mode = Unknown ) : m_Mode( _Mode )
            {
            }
            void SetMode( EAgentMode_t _Mode, const SOptions_t &_data )
            {
                m_Mode = _Mode;
                RefreshAgent( _data );
            }
            void Start() throw( std::exception )
            {
                m_Agent->Start();
            }
            EAgentMode_t getMode()
            {
                return m_Mode;
            }
            EExitCodes_t getExitCode() const
            {
                return m_Agent->getExitCode();
            }

        private:
            void RefreshAgent( const SOptions_t &_data )
            {
                if(( m_Agent.get() && m_Agent->GetMode() != m_Mode ) || !m_Agent.get() )
                    m_Agent.reset( Spawn( _data ) );
            }
            CAgentBase* Spawn( const SOptions_t &_data )
            {
                switch( m_Mode )
                {
                    case Server:
                        return new CAgentServer( _data );
                    case Client:
                        return new CAgentClient( _data );
                    case Unknown:
                        return NULL;
                }
                return NULL;
            }

        private:
            pAgentBase_t m_Agent;
            EAgentMode_t m_Mode;
    };

};

#endif
