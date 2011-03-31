//
//  Server.h
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef CLIENT_POD_REMOTE_H
#define CLIENT_POD_REMOTE_H
//=============================================================================
//STD
#include <string>
// PoD Protocol
#include "ProtocolCommands.h"
//=============================================================================
namespace pod_remote
{
    class CClient
    {
            enum Requests { Req_ShutDown, Req_Host_Info };
        public:
            CClient( int _fdIn, int _fdOut );
            void getSrvHostInfo( PROOFAgent::SHostInfoCmd *_srvHostInfo ) const;

        private:
            void processAdminConnection( MiscCommon::BYTEVector_t *_data,
                                         CClient::Requests _req ) const;
            int processProtocolMsgs( MiscCommon::BYTEVector_t *_data,
                                     PROOFAgent::CProtocol * _protocol,
                                     CClient::Requests _req ) const;

        private:
            int m_fdIn;
            int m_fdOut;
    };
}
#endif
