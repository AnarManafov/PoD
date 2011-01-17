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
//=============================================================================
namespace pod_info
{
    class CServer
    {
        public:
            CServer( const std::string &_host, int _port );

        private:
            std::string m_host;
            int m_port;
    };
}
#endif