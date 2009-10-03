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
// MiscCommon
#include "LogImp.h"
// PROOFAgent
#include "PROOFCfgImpl.h"
#include "PFContainer.h"
#include "AgentBase.h"
#include "NewPacketForwarder.h"
//=============================================================================

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
            typedef std::set<MiscCommon::INet::Socket_t> Sockets_type;
        public:
            CAgentServer( const SOptions_t &_data );
            virtual ~CAgentServer();

            REGISTER_LOG_MODULE( "AgentServer" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }
            void AddPF( MiscCommon::INet::Socket_t _ClientSocket,
                        unsigned short _nNewLocalPort,
                        const std::string &_sPROOFCfgString );
            void CleanDisconnectsPF( const std::string &_sPROOFCfg );

        protected:
            void ThreadWorker();

        private:
            void deleteServerInfoFile();
            void createClientNode( MiscCommon::INet::smart_socket &_sock );
            void mainSelect( const MiscCommon::INet::CSocketServer &_server );

        private:
            MiscCommon::INet::Socket_t f_serverSocket;
            CNodeContainer m_nodes;
            Sockets_type m_socksToSelect;
            CNewPacketForwarder m_packetForwarder;

            PoD::SServerOptions_t m_Data;
            CPFContainer m_PFList;
            boost::mutex m_PFList_mutex;
            std::string m_serverInfoFile;
    };

}

#endif /* AGENTSERVER_H_ */
