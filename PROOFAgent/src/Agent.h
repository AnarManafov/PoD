/************************************************************************/
/**
 * @file Agent.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENT_H
#define AGENT_H

// STD
#include <memory>

// OUR
#include "AgentImpl.h"

namespace PROOFAgent
{
    typedef std::auto_ptr<CAgentBase> pAgentBase_t;
    class CAgent
    {
        public:
            CAgent( EAgentMode_t _Mode = Unknown ) : m_Mode( _Mode )
            {
                RefreshAgent ();
            }
            void SetMode( EAgentMode_t _Mode )
            {
                m_Mode = _Mode;
                RefreshAgent ();
            }
            MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element )
            {
                return m_Agent->Init( _element );
            }
            MiscCommon::ERRORCODE Start()
            {
                return m_Agent->Start();
            }

        private:
            void RefreshAgent ()
            {
                if ( ( m_Agent.get() && m_Agent->Mode != m_Mode ) || !m_Agent.get() )
                    m_Agent.reset( Spawn() );
            }
            CAgentBase* Spawn()
            {
                switch ( m_Mode )
                {
                    case Server:
                        return new CAgentServer;
                        break;
                    case Client:
                        return new CAgentClient;
                        break;
                    case Unknown:
                        return NULL;
                        break;
                }
                return NULL;
            }

        private:
            pAgentBase_t m_Agent;
            EAgentMode_t m_Mode;
    };

};

#endif
