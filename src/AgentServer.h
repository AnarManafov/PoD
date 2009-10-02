/************************************************************************/
/**
 * @file AgentServer.h
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTSERVER_H_
#define AGENTSERVER_H_
// BOOST
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "LogImp.h"
// PROOFAgent
#include "PROOFCfgImpl.h"
#include "PFContainer.h"
#include "AgentBase.h"


namespace PROOFAgent
{

//=============================================================================
    /**
     *
     * @brief An agent class, for the server mode of PROOFAgent
     *
     */
    class CAgentServer :
            public CAgentBase,
            MiscCommon::CLogImp<CAgentServer>,
            protected CPROOFCfgImpl<CAgentServer>

    {
        public:
            CAgentServer( const SOptions_t &_data ): CAgentBase( _data.m_podOptions.m_server.m_common )
            {
                m_Data = _data.m_podOptions.m_server;
                m_serverInfoFile = _data.m_serverInfoFile;

                //InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
            }
            virtual ~CAgentServer()
            {
                deleteServerInfoFile();
            }
            REGISTER_LOG_MODULE( "AgentServer" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }

            void AddPF( MiscCommon::INet::Socket_t _ClientSocket,
                        unsigned short _nNewLocalPort,
                        const std::string &_sPROOFCfgString )
            {
                boost::mutex::scoped_lock lock( m_PFList_mutex );
                m_PFList.add( _ClientSocket, _nNewLocalPort, _sPROOFCfgString );
            }

            void CleanDisconnectsPF( const std::string &_sPROOFCfg )
            {
                m_PFList.clean_disconnects( _sPROOFCfg );
            }

        protected:
            void ThreadWorker();

        private:
            void deleteServerInfoFile();

        private:
            PoD::SServerOptions_t m_Data;
            CPFContainer m_PFList;
            boost::mutex m_PFList_mutex;
            std::string m_serverInfoFile;
    };

}

#endif /* AGENTSERVER_H_ */
