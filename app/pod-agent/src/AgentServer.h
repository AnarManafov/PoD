/************************************************************************/
/**
 * @file AgentServer.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTSERVER_H_
#define AGENTSERVER_H_
// MiscCommon
#include "LogImp.h"
// pod-agent
#include "Node.h"
#include "ThreadPool.h"
#include "AgentBase.h"
#include "Protocol.h"
#include "ProtocolCommands.h"
//=============================================================================
namespace PROOFAgent
{
    typedef std::queue<ECmdType> requests_t;
    struct SWorkerInfo
    {
        SWorkerInfo( const std::string &_infoString ):
            m_proofPort( 0 ),
            m_removeMe( false ),
            m_id( 0 ),
            m_numberOfPROOFWorkers( 1 ),
            m_bupInfoString( _infoString )
        {
        }
        std::string string() const
        {
            std::stringstream ss;
            if( 0 != m_id )
            {
                ss
                        << m_user << "@" << m_host << ":" << m_proofPort
                        << "[id:" << m_id << "]";
            }
            else
            {
                ss << m_bupInfoString;
            }

            return ss.str();
        }
        CProtocol m_protocol;
        std::string m_host;
        std::string m_user;
        uint16_t m_proofPort;
        std::string m_proofCfgEntry;
        bool m_removeMe;
        uint32_t m_id;
        requests_t m_requests;
        unsigned int m_numberOfPROOFWorkers;
        std::string m_bupInfoString;
    };

    typedef std::pair<int, SWorkerInfo> wrkValue_t;
    typedef std::list<wrkValue_t> workersMap_t;
//=============================================================================
    /**
     *
     * @brief The agent class interface, for the server mode of PROOFAgent
     *
     */
    class CAgentServer :
        public CAgentBase,
        MiscCommon::CLogImp<CAgentServer>
    {
            typedef boost::shared_ptr<CNode> node_type;
            typedef std::list<node_type> Sockets_type;
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
            void log( MiscCommon::LOG_SEVERITY _Severity, const std::string &_msg )
            {
                msgPush( _Severity, _msg );
            }

        private:
            void deleteServerInfoFile() const;
            void createClientNode( workersMap_t::value_type &_wrk );
            int prepareFDSet( fd_set *_readset );
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
                                                   unsigned short _Port,
                                                   const std::string &_RealWrkHost,
                                                   bool usePF,
                                                   unsigned int _numberOfPROOFWorkers = 1 );
            void updatePROOFCfg();
            void processAdminConnection( workersMap_t::value_type &_wrk );
            void processProtocolMsgs( workersMap_t::value_type &_wrk );
            void setupPROOFWorker( workersMap_t::value_type &_wrk );
            void usePacketForwarding( workersMap_t::value_type &_wrk );
            void sendServerRequest( workersMap_t::value_type &_wrk );
            void createServerInfoFile();
            void shutdownWNs();
        
        private:
            MiscCommon::INet::Socket_t f_serverSocket;
            Sockets_type m_socksToSelect;
            PoD::SServerOptions_t m_Data;
            CThreadPool m_threadPool;
            std::string m_serverInfoFile;
            std::string m_masterEntryInPROOFCfg;
            workersMap_t m_adminConnections; // the map of workers, which are connected to admin channel
            uint32_t m_workerMaxID;
            unsigned int m_agentServerListenPort;
    };

}

#endif /* AGENTSERVER_H_ */
