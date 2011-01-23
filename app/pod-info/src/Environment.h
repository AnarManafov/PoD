//
//  Environment.h
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
//=============================================================================
#include <string>
//=============================================================================
class CEnvironment
{
    public:
        CEnvironment();

    public:
        void init();
        void checkRemoteServer( const std::string &_cfg );
        std::string version() const
        {
            return m_localVer;
        }
        std::string PoDPath() const
        {
            return m_PoDPath;
        }
        bool isLocalServer() const
        {
            return m_isLocalServer;
        }
        std::string serverHost() const
        {
            return m_srvHost;
        }
        unsigned int serverPort() const
        {
            return m_srvPort;
        }

    private:
        void getLocalVersion();
        bool checkForLocalServer();
        void readServerInfoCfg( std::ifstream &_f );

    private:
        std::string m_PoDPath;
        std::string m_localVer;
        bool m_isLocalServer;
        std::string m_srvHost;
        unsigned int m_srvPort;
};

#endif
