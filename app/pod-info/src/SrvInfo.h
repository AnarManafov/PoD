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
#include <iostream>
//=============================================================================
class CEnvironment;
namespace pod_info
{
    class CServer;
}
//=============================================================================
class CSrvInfo
{
    public:
        enum ESrvStatus {srvStatus_OK, srvStatus_Down, srvStatus_NeedRestart };

    public:
        CSrvInfo( const CEnvironment *_env );

    public:
        void getInfo( pod_info::CServer * _agentServer = NULL );
        void printInfo( std::ostream &_stream ) const;
        void printConnectionString( std::ostream &_stream ) const;
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
        const CEnvironment *m_env;
        std::string m_serverUsername;
};

#endif
