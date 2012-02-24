//
//  SrvInfo.cpp
//  PoD
//
//  Created by Anar Manafov on 03.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "SrvInfo.h"
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// pod-agent
#include "ProofStatusFile.h"
#include "ProtocolCommands.h"
// MiscCommon
#include "Process.h"
#include "PoDSysFiles.h"
// pod-info
#include "Server.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
namespace pod_agent = PROOFAgent;
namespace bpo = boost::program_options;
//=============================================================================
CSrvInfo::CSrvInfo( const CPoDEnvironment *_env ):
    m_xpdPid( 0 ),
    m_agentPid( 0 ),
    m_xpdPort( 0 ),
    m_agentPort( 0 ),
    m_env( _env ),
    m_srvHost( "localhost" )
{
}
//=============================================================================
void CSrvInfo::localXPDInfo()
{
    try
    {
        // Obtain information about a local xpd process
        pod_agent::CProofStatusFile proofStatus;
        if( proofStatus.readAdminPath( m_env->xpdCfgFile(), adminp_server ) )
        {
            m_xpdPid = proofStatus.xpdPid();
            m_xpdPort = proofStatus.xpdPort();
            if( !IsProcessExist( m_xpdPid ) )
            {
                m_xpdPid = 0;
                m_xpdPort = 0;
                return;
            }
        }
    }
    catch( ... )
    {
    }
}
//=============================================================================
void CSrvInfo::localAgentInfo()
{
    ifstream f( m_env->agentPidFile().c_str() );
    if( !f.is_open() )
    {
        m_agentPid = 0;
        m_agentPort = 0;
        return;
    }

    f >> m_agentPid;
    if( !IsProcessExist( m_agentPid ) )
    {
        m_agentPid = 0;
        m_agentPort = 0;
    }
}
//=============================================================================
void CSrvInfo::remoteCombinedInfo( pod_info::CServer *_agentServer )
{
    pod_agent::SHostInfoCmd srvHostInfo;
    try
    {
        _agentServer->getSrvHostInfo( &srvHostInfo );
    }
    catch( exception &_e )
    {
        m_xpdPid = 0;
        m_xpdPort = 0;
        m_agentPid = 0;
        m_agentPort = 0;
        return;
    }

    m_xpdPid = srvHostInfo.m_xpdPid;
    m_xpdPort = srvHostInfo.m_xpdPort;
    m_agentPid = srvHostInfo.m_agentPid;
    m_agentPort = srvHostInfo.m_agentPort;
    m_serverUsername = srvHostInfo.m_username;
}
//=============================================================================
void CSrvInfo::getInfo( pod_info::CServer * _agentServer )
{
    if( NULL == _agentServer )
    {
        localXPDInfo();
        localAgentInfo();
        get_cuser_name( &m_serverUsername );
    }
    else
    {
        remoteCombinedInfo( _agentServer );
    }
}
//=============================================================================
void CSrvInfo::printAgent( ostream &_stream ) const
{
    if( m_agentPid )
    {
        _stream << "PoD agent [" << m_agentPid << "] port: " << m_agentPort << endl;
    }
    else
    {
        _stream << "PoD agent is NOT running." << endl;
    }
}
//=============================================================================
void CSrvInfo::printXpd( ostream &_stream ) const
{
    if( m_xpdPid )
    {
        _stream << "XPROOFD [" << m_xpdPid << "] port: " << m_xpdPort << endl;
    }
    else
    {
        _stream << "XPROOFD is NOT running." << endl;
    }
}
//=============================================================================
void CSrvInfo::printInfo( ostream &_stream ) const
{
    switch( getStatus() )
    {
        case srvStatus_Down:
            _stream << "PoD server is NOT running." << endl;
            break;
        case srvStatus_NeedRestart:
            {
                printXpd( _stream );
                printAgent( _stream );
                _stream
                        << "WARNING: Missing services detected. Please, restart PoD server."
                        << endl;
            }
            break;
        case srvStatus_OK:
            {
                printXpd( _stream );
                printAgent( _stream );
                // pod-remote servers need to be treated differently
#if defined (BOOST_PROPERTY_TREE)
                PoD::SPoDRemoteOptions opt_file;
                if( processPoDRemoteCfg( &opt_file ) )
                {
                    _stream
                            << "PoD agent port is forwarded via local port: " << opt_file.m_localAgentPort << "\n"
                            << "XPROOFD port is forwarded via local port: " << opt_file.m_localXpdPort << endl;
                }
#endif
            }
            break;
    }
}
//=============================================================================
void CSrvInfo::printConnectionString( ostream &_stream ) const
{
    if( getStatus() == srvStatus_OK )
    {
        // pod-remote servers need to be treated differently
#if defined (BOOST_PROPERTY_TREE)
        PoD::SPoDRemoteOptions opt_file;
        if( processPoDRemoteCfg( &opt_file ) )
            _stream << m_serverUsername << "@" << m_srvHost << ":" << opt_file.m_localXpdPort << endl;
        else
            _stream << m_serverUsername << "@" << m_srvHost << ":" << m_xpdPort << endl;
#else
        _stream << m_serverUsername << "@" << m_srvHost << ":" << m_xpdPort << endl;
#endif
    }
}
//=============================================================================
bool CSrvInfo::processServerInfoCfg( const string *_cfg )
{
    string filename( m_env->srvInfoFile() );
    if( NULL != _cfg )
        filename = *_cfg;

    ifstream f( filename.c_str() );
    if( !f.is_open() )
        return false;

    // read server info file
    bpo::variables_map keys;
    bpo::options_description options(
        "Agent's server info config" );
    options.add_options()
    ( "server.host", bpo::value<string>(), "" )
    ( "server.port", bpo::value<unsigned int>(), "" )
    ;

    // Parse the config file
    bpo::store( bpo::parse_config_file( f, options ), keys );
    bpo::notify( keys );
    if( keys.count( "server.host" ) )
        m_srvHost = keys["server.host"].as<string> ();
    if( keys.count( "server.port" ) )
        m_agentPort = keys["server.port"].as<unsigned int> ();

    return true;
}
//=============================================================================
#if defined (BOOST_PROPERTY_TREE)
bool CSrvInfo::processPoDRemoteCfg( PoD::SPoDRemoteOptions *_opt_file ) const
{
    if( !m_env || !_opt_file )
        return false;
    pid_t podRemotePid = CPIDFile::GetPIDFromFile( m_env->pod_remotePidFile() );
    if( podRemotePid > 0 && IsProcessExist( podRemotePid ) )
    {
        _opt_file->load( m_env->pod_remoteCfgFile() );
        return true;
    }
    return false;
}
#endif
