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
            MiscCommon::CLogImp<CAgentServer>
    {
            typedef std::list<MiscCommon::INet::Socket_t> Sockets_type;
        public:
            CAgentServer( const SOptions_t &_data );
            virtual ~CAgentServer();

            REGISTER_LOG_MODULE( "AgentServer" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }

        protected:
            void run();
            void monitor();

        private:
            void deleteServerInfoFile();
            void createClientNode( MiscCommon::INet::smart_socket &_sock );
            void mainSelect( const MiscCommon::INet::CSocketServer &_server );
            void createPROOFCfg();
            /**
              *
              * @brief This method creates proof.conf entries for nodes
              * @note
              example of proof.conf for server
              @verbatim

              master depc218.gsi.de  workdir=~/proof
              worker manafov@localhost port=20001 perf=100 workdir=~/

              @endverbatim
              example of proof.conf for client
              @verbatim

              master lxial24.gsi.de
              worker lxial24.gsi.de perf=100

              @endverbatim
              *
              */
            std::string createPROOFCfgEntryString( const std::string &_UsrName,
                                                   unsigned short _Port, const std::string &_RealWrkHost );
            void updatePROOFCfg();

        private:
            MiscCommon::INet::Socket_t f_serverSocket;
            CNodeContainer m_nodes;
            Sockets_type m_socksToSelect;
            CThreadPool m_threadPool;
            PoD::SServerOptions_t m_Data;
            std::string m_serverInfoFile;
            std::string m_masterEntryInPROOFCfg;
            int m_fdSignalPipe;
    };

}

#endif /* AGENTSERVER_H_ */
