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
#include "AgentImpl.h"

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
            friend class boost::serialization::access;
        public:
            CAgent( EAgentMode_t _Mode = Unknown ) : m_Mode( _Mode )
            {
                RefreshAgent();
            }
            void SetMode( EAgentMode_t _Mode )
            {
                m_Mode = _Mode;
                RefreshAgent();
            }
            void Start( const std::string &_PROOFCfg ) throw( std::exception )
            {
                m_Agent->Start( _PROOFCfg );
            }

        private:
            void RefreshAgent()
            {
                if (( m_Agent.get() && m_Agent->GetMode() != m_Mode ) || !m_Agent.get() )
                    m_Agent.reset( Spawn() );
            }
            CAgentBase* Spawn()
            {
                switch ( m_Mode )
                {
                    case Server:
                        return new CAgentServer;
                    case Client:
                        return new CAgentClient;
                    case Unknown:
                    default:
                        return NULL;
                }
            }
            template<class Archive>
            void save( Archive & _ar, const unsigned int /*_version*/ ) const
            {
                const CAgentBase * const p = m_Agent.get();
                _ar & BOOST_SERIALIZATION_NVP( p );
            }
            template<class Archive>
            void load( Archive & _ar, const unsigned int /*_version*/ )
            {
                CAgentBase *p = m_Agent.get();
                _ar & BOOST_SERIALIZATION_NVP( p );
            }
            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            pAgentBase_t m_Agent;
            EAgentMode_t m_Mode;
    };

};
BOOST_CLASS_VERSION( PROOFAgent::CAgent, 1 )

#endif
