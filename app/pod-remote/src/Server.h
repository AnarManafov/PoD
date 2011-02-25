//
//  Server.h
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef SERVER_H
#define SERVER_H
//=============================================================================
//STD
#include <string>
// MiscCommon
#include "INet.h"
// pod-agent
#include "Protocol.h"
#include "ProtocolCommands.h"
//=============================================================================
namespace inet = MiscCommon::INet;
//=============================================================================
namespace pod_info
{
    class CServer
    {
            enum Requests { Req_Host_Info, Req_WNs_List };
        public:
            CServer( const std::string &_host, unsigned int _port );
            void getSrvHostInfo( PROOFAgent::SHostInfoCmd *_srvHostInfo ) const;
            void getListOfWNs( PROOFAgent::SWnListCmd *_lst ) const;

        private:
            void processAdminConnection( MiscCommon::BYTEVector_t *_data,
                                         int _serverSock, CServer::Requests _req ) const;
            int processProtocolMsgs( MiscCommon::BYTEVector_t *_data, int _serverSock,
                                     PROOFAgent::CProtocol * _protocol,
                                     CServer::Requests _req ) const;

        private:
            std::string m_host;
            unsigned int m_port;
    };
}
#endif
