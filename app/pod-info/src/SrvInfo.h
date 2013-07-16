//
//  SrvInfo.h
//  PoD
//
//  Created by Anar Manafov on 03.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef SRVINFO_H
#define SRVINFO_H
//=============================================================================
#include <stdint.h>
#include <iostream>
// PoD
#include "PoDUserDefaultsOptions.h"
//=============================================================================
class CPoDEnvironment;
namespace pod_info
{
    class CServer;
}
//=============================================================================
enum EPoDServerType
{
    // PoD Server can't be found
    SrvType_Unknown,
    // a local PoD server.
    SrvType_Local,
    // a remote PoD server
    SrvType_Remote,
    // a remote PoD server, start by pod-remote
    SrvType_RemoteManaged
};
//=============================================================================
class CSrvInfo
{
    public:
        enum ESrvStatus {srvStatus_OK, srvStatus_Down, srvStatus_NeedRestart };

    public:
        CSrvInfo( const CPoDEnvironment *_env );

    public:
        void getInfo( pod_info::CServer * _agentServer = NULL );
        void printInfo( std::ostream &_stream ) const;
        void printConnectionString( std::ostream &_stream ) const;
        bool processServerInfoCfg( const std::string *_cfg = NULL );
#if defined (BOOST_PROPERTY_TREE)
        bool processPoDRemoteCfg( PoD::SPoDRemoteOptions *_opt_file ) const;
#endif
        ESrvStatus getStatus() const
        {
            if( 0 == m_agentPid && 0 == m_xpdPid )
                return srvStatus_Down;
            else if( 0 != m_agentPid && 0 != m_xpdPid )
                return srvStatus_OK;
            else
                return srvStatus_NeedRestart;
        }
        pid_t xpdPid() const
        {
            return m_xpdPid;
        }
        pid_t agentPid() const
        {
            return m_agentPid;
        }
        uint16_t xpdPort() const
        {
            return m_xpdPort;
        }
        uint16_t agentPort() const
        {
            return m_agentPort;
        }
        std::string serverHost() const
        {
            return m_srvHost;
        }

    private:
        void localXPDInfo();
        void localAgentInfo();
        void remoteCombinedInfo( pod_info::CServer * _agentServer );
        void printAgent( std::ostream &_stream ) const;
        void printXpd( std::ostream &_stream ) const;

    private:
        pid_t m_xpdPid;
        pid_t m_agentPid;
        uint16_t m_xpdPort;
        uint16_t m_agentPort;
        const CPoDEnvironment *m_env;
        std::string m_serverUsername;
        std::string m_srvHost;
        std::string m_srvUser;
};

#endif
