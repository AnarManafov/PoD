/************************************************************************/
/**
 * @file AgentClient.h
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H
// BOOST
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "LogImp.h"
#include "SysHelper.h"
// PROOFAgent
#include "PacketForwarder.h"
#include "AgentBase.h"

//=============================================================================
namespace PROOFAgent
{
//=============================================================================
    /**
     *
     * @brief An agent class, for the client mode of PROOFAgent
     *
     */
    class CAgentClient:
            public CAgentBase,
            MiscCommon::CLogImp<CAgentClient>
    {
        public:
            CAgentClient( const SOptions_t &_data ): CAgentBase( _data.m_podOptions.m_worker.m_common )
            {
                m_Data = _data.m_podOptions.m_worker;
                m_serverInfoFile = _data.m_serverInfoFile;
                m_proofPort = _data.m_proofPort;

                //InfoLog( MiscCommon::erOK, "Agent Client configuration:" ) << m_Data;
            }
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( "AgentClient" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void run();
            void monitor();

        private:
            PoD::SWorkerOptions_t m_Data;
            std::string m_serverInfoFile;
            unsigned int m_proofPort;
    };

}

#endif
