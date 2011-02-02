/************************************************************************/
/**
 * @file main.cpp
 * @brief main file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2011-01-17
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2011 GSI, Scientific Computing devision. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// MiscCommon
#include "BOOSTHelper.h"
#include "Process.h"
#include "SysHelper.h"
#include "PoDUserDefaultsOptions.h"
// pod-agent
#include "ProofStatusFile.h"
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
#include "Options.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace pod_agent = PROOFAgent;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
LPCTSTR g_remote_server_info_cfg = "~/.PoD/etc/remote_server_info.cfg";
LPCTSTR g_ssh_tunnel_pid = "~/.PoD/etc/server_tunnel.pid";
LPCTSTR g_xpdCFG = "etc/xpd.cf";
// 0 - success, 1 - an error, 2 - server is partially running
int g_exitCode = 0;
//=============================================================================
string version( const CEnvironment &_env, const pod_info::CServer &_srv )
{
    bool noServer( false );
    PROOFAgent::SHostInfoCmd srvHostInfo;
    try
    {
        _srv.getSrvHostInfo( &srvHostInfo );
    }
    catch( ... )
    {
        noServer = true;
    }
    ostringstream ss;
    ss
            << "PoD location: " << _env.PoDPath() << "\n"
            << "Local Version: " << _env.version() << "\n";

    if( noServer )
    {
        ss << "PoD server is NOT found.";
    }
    else
    {
        ss
                << "Server PoD location: "
                << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
                << srvHostInfo.m_PoDPath << "\n"
                << "Server Version: " << srvHostInfo.m_version;
    }
    return ( ss.str() );
}
//=============================================================================
size_t listWNs( string *_output, const pod_info::CServer &_srv,
                const SOptions &_opt )
{
    PROOFAgent::SWnListCmd lst;
    try
    {
        _srv.getListOfWNs( &lst );
    }
    catch( exception &_e )
    {
        string msg;
        msg += "PoD server is NOT found.\n";
        if( _opt.m_debug )
            msg += _e.what();
        throw runtime_error( msg );
    }
    if( _output )
    {
        stringstream ss;
        std::ostream_iterator< std::string > output( ss, "\n" );
        std::copy( lst.m_container.begin(), lst.m_container.end(), output );
        *_output = ss.str();
    }
    return ( lst.m_container.size() );
}
//=============================================================================
// check the local xpd
pid_t getLocalXPDPid()
{
    try
    {
        // PoD user defaults
        string pathUD( PoD::showCurrentPUDFile() );
        smart_path( &pathUD );
        PoD::CPoDUserDefaults user_defaults;
        user_defaults.init( pathUD );
        PoD::SPoDUserDefaultsOptions_t cfg( user_defaults.getOptions() );

        string xpd( cfg.m_server.m_common.m_workDir );
        smart_append( &xpd, '/' );
        xpd += g_xpdCFG;
        smart_path( &xpd );
        pod_agent::CProofStatusFile proofStatus;
        if( proofStatus.readAdminPath( xpd, adminp_server ) )
            return proofStatus.xpdPid();
    }
    catch( ... )
    {
    }
    return 0;
}
//=============================================================================
void srvPoDStatus( string *_status, string *_con_string,
                   const pod_info::CServer &_srv, const SOptions &_opt )
{
    PROOFAgent::SHostInfoCmd srvHostInfo;
    try
    {
        _srv.getSrvHostInfo( &srvHostInfo );
    }
    catch( exception &_e )
    {
        string msg;
        if( _status && _opt.m_sshConnectionStr.empty() )
        {
            // if we are here, that means there is no neither
            // a remote or a local pod-agent found.
            // Let us check now, whether there is no local xpd processes left
            pid_t xpd_pid( getLocalXPDPid() );
            if( IsProcessExist( xpd_pid ) )
            {
                stringstream ss;
                ss << "PoD server is NOT running.\n"
                   << "WARNING: There is a local XPROOFD [" << xpd_pid << "] process detected as a part of PoD server.\n"
                   << "Please, either restart PoD server or stop it explicitly: \"pod-server stop\".";
                msg += ss.str();
                g_exitCode = 2;
                throw runtime_error( msg );
            }
        }

        msg += "PoD server is NOT found.\n";
        if( _opt.m_debug )
            msg += _e.what();

        throw runtime_error( msg );
    }

    if( _status )
    {
        ostringstream ss;
        ss
                << "XPROOFD [" << srvHostInfo.m_xpdPid << "] port: " << srvHostInfo.m_xpdPort
                << "\nPoD agent [" << srvHostInfo.m_agentPid << "] port: " << srvHostInfo.m_agentPort;
        *_status = ss.str();
    }

    if( _con_string )
    {
        ostringstream ss;
        ss
                << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
                << srvHostInfo.m_xpdPort;
        *_con_string = ss.str();
    }

}
//=============================================================================
pid_t tunnelPid()
{
    string name( g_ssh_tunnel_pid );
    smart_path( &name );
    ifstream f( name.c_str() );
    if( !f.is_open() )
        return 0;
    pid_t tunnel_pid;
    f >> tunnel_pid;
    return tunnel_pid;
}
//=============================================================================
void killTunnel()
{
    pid_t pid = tunnelPid();
    if( 0 != pid )
        kill( pid, SIGKILL );

    string name( g_ssh_tunnel_pid );
    smart_path( &name );
    unlink( name.c_str() );
}
//=============================================================================
int main( int argc, char *argv[] )
{
    CEnvironment env;
    try
    {
        env.init();

        SOptions options;
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        string srvHost( env.serverHost() );
        // use SSH to retrieve server_info.cfg
        if( !options.m_sshConnectionStr.empty() )
        {
            string outfile( g_remote_server_info_cfg );
            smart_path( &outfile );

            // delete first the remote srv info file
            unlink( outfile.c_str() );
            StringVector_t arg;
            arg.push_back( "-l " + options.m_sshConnectionStr );
            arg.push_back( "-f " + outfile );
            if( options.m_debug )
                arg.push_back( "-d" );
            if( options.m_batchMode )
                arg.push_back( "-b" );
            string cmd( "$POD_LOCATION/bin/private/pod-srv-info" );
            smart_path( &cmd );
            string stdout;
            do_execv( cmd, arg, 60, NULL );
            if( !does_file_exists( outfile ) )
            {
                stringstream ss;
                ss << "Remote PoD server is NOT running.";
                throw runtime_error( ss.str() );
            }

            env.checkRemoteServer( outfile );
            // now we can delete the remote server file
            // we can't reuse it in next sessions, since the PoD port could change
            unlink( outfile.c_str() );

            // delete tunnel's file
            killTunnel();
            // create an ssh tunnel on PoD Server port
            switch( fork() )
            {
                case - 1:
                    // Unable to fork
                    throw std::runtime_error( "Unable to create an SSH tunnel." );
                case 0:
                    {
                        // create SSH Tunnel
                        string cmd( "$POD_LOCATION/bin/private/pod-ssh-tunnel" );
                        smart_path( &cmd );
                        string l_arg( "-l" );
                        l_arg += options.m_sshConnectionStr;
                        stringstream p_arg;
                        p_arg << "-p" << env.serverPort();
                        string o_arg( "-o" );
                        o_arg += options.m_openDomain;

                        string sBatch;
                        if( options.m_batchMode )
                            sBatch = "-b";

                        execl( cmd.c_str(), "pod-ssh-tunnel",
                               l_arg.c_str(), p_arg.str().c_str(),
                               o_arg.c_str(), sBatch.c_str(), NULL );
                        exit( 1 );
                    }
            }
            // wait for tunnel to start
            short count( 0 );
            const short max_try( 50 );
            pid_t pid( tunnelPid() );
            while( 0 == pid || !IsProcessExist( pid ) ||
                   0 != MiscCommon::INet::get_free_port( env.serverPort() ) )
            {
                ++count;
                pid = tunnelPid();
                if( count >= max_try )
                    throw runtime_error( "Can't setup an SSH tunnel." );
                usleep( 50000 ); // delays for 0.05 seconds
            }
            // we tunnel the connection to PoD server
            srvHost = "localhost";
        }

        if( options.m_debug )
        {
            cout
                    << "connecting to PoD server: "
                    << srvHost << ":" << env.serverPort() << endl;
        }
        pod_info::CServer srv( srvHost, env.serverPort() );

        // Show version information
        if( options.m_version )
        {
            cout << version( env, srv ) << endl;
            killTunnel();
            return 0;
        }

        // PoD Server status
        if( options.m_status || options.m_connectionString )
        {
            string status;
            string con_string;
            srvPoDStatus( &status, &con_string, srv, options );
            if( options.m_status )
            {
                cout << status << endl;
            }
            if( options.m_connectionString )
            {
                cout << con_string << endl;
            }
        }

        // list of and a number of available WNs
        if( options.m_countWNs || options.m_listWNs )
        {
            string lst;
            size_t n = listWNs(( options.m_listWNs ? &lst : NULL ), srv, options );

            if( options.m_countWNs )
            {
                cout << n << endl;
            }
            if( options.m_listWNs )
            {
                cout << lst;
                cout.flush();
            }
        }
    }
    catch( exception& e )
    {
        killTunnel();
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return g_exitCode;
    }
    killTunnel();
    return g_exitCode;
}
