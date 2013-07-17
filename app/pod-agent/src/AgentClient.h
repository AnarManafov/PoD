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

        Copyright (c) 2007-2011 GSI, Scientific Computing division. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H
// MiscCommon
#include "LogImp.h"
#include "INet.h"
// PROOFAgent
#include "AgentBase.h"
//=============================================================================
namespace PROOFAgent
{
    class CNode;
    class CProtocol;
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
            CAgentClient( const SOptions_t &_data );
            virtual ~CAgentClient();
            REGISTER_LOG_MODULE( "AgentClient" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void run();
            void monitor();
            void log( MiscCommon::LOG_SEVERITY _Severity, const std::string &_msg )
            {
                msgPush( _Severity, _msg );
            }

        private:
            void waitForServerToConnect( MiscCommon::INet::Socket_t _sockToWait );
            // returns a connection socket to a local PROOF worker
            MiscCommon::INet::Socket_t connectToLocalPROOF( unsigned int _proofPort );
            void mainSelect( CNode *_node );
            void createPROOFCfg();
            void processAdminConnection( int _serverSock );
            int processProtocolMsgs( int _serverSock, CProtocol * _protocol );
            void readServerInfoFile( const std::string &_filename );

        private:
            PoD::SWorkerOptions_t m_Data;
            PoD::SServerOptions_t m_ServerData;
            std::string m_serverInfoFile;
            uint32_t m_id;
            bool m_isDirect;
            uint32_t m_numberOfPROOFWorkers;
            unsigned int m_agentServerListenPort;
            std::string m_agentServerHost;
            std::string m_agentServerUser;
    };

}

#endif
