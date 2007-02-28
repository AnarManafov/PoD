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
#ifndef PROOFAGENTAGENT_H
#define PROOFAGENTAGENT_H

#include "AgentImpl.h"

namespace proofagent
{

    template <typename _AgentTraits = CAgentServer>
    class CAgent
    {
        public:
            ERRORCODE Init()
            {
                return m_Agent.Init();
            }

        private:
            _AgentTraits m_Agent;
    };

}

#endif
