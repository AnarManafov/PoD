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
// STD
#include <string>
// MiscCommon
#include "PoDUserDefaultsOptions.h"
//=============================================================================
class CEnvironment
{
    public:
        CEnvironment();
        ~CEnvironment();

    public:
        void init();
        bool processServerInfoCfg( const std::string *_cfg = NULL );

        std::string version() const
        {
            return m_localVer;
        }
        std::string PoDPath() const
        {
            return m_PoDPath;
        }
        std::string serverHost() const
        {
            return m_srvHost;
        }
        unsigned int serverPort() const
        {
            return m_srvPort;
        }
        const PoD::SPoDUserDefaultsOptions_t getUD() const
        {
            return *m_ud;
        }
        std::string localSrvInfoFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/server_info.cfg";
            return ret;
        }
        std::string remoteSrvInfoFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/remote_server_info.cfg";
            return ret;
        }
        std::string getTunnelPidFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "server_tunnel.pid";
            return ret;
        }
        std::string getXpdCfgFile() const
        {
            if( m_wrkDir.empty() )
                return( "" );

            std::string ret( m_wrkDir );
            ret += "etc/xpd.cf";
            return ret;
        }

    private:
        void getLocalVersion();

    private:
        std::string m_PoDPath;
        std::string m_localVer;
        std::string m_srvHost;
        unsigned int m_srvPort;
        PoD::SPoDUserDefaultsOptions_t *m_ud;
        std::string m_wrkDir;
};

#endif
