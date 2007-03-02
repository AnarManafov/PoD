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
#ifndef AGENT_H
#define AGENT_H

#include "AgentImpl.h"
#include <memory>

namespace PROOFAgent
{
    typedef std::auto_ptr<CAgentBase*> pAgentBase_t;
    class CAgent
    {
        public:
            CAgent( EAgentMode_t _Mode = Unknown ) : m_Mode( _Mode )
            {}
            void SetMode( EAgentMode_t _Mode )
            {
                m_Mode = _Mode;
            }
            ERRORCODE Init()
            {
                return erNotImpl; //m_Agent->Init();
            }

        private:
            void GetAgent ()
            {
                //  if( m_Agent->get() &&  )

            }

        private:
            pAgentBase_t m_Agent;
            EAgentMode_t m_Mode;
    };

};

#endif
