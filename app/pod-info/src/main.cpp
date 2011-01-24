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
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
LPCTSTR g_remote_server_info_cfg = "~/.PoD/etc/remote_server_info.cfg";
LPCTSTR g_ssh_tunnel_pid = "~/.PoD/etc/server_tunnel.pid";
//=============================================================================
struct SOptions
{
    SOptions():
        m_version( false ),
        m_connectionString( false ),
        m_listWNs( false ),
        m_countWNs( false ),
        m_status( false ),
        m_debug( false ),
        m_batchMode( false )
    {
    }
    bool operator== ( const SOptions &_val )
    {
        return ( m_version == _val.m_version &&
                 m_connectionString == _val.m_connectionString &&
                 m_listWNs == _val.m_listWNs &&
                 m_countWNs == _val.m_countWNs &&
                 m_status == _val.m_status &&
                 m_debug == _val.m_debug &&
                 m_sshConnectionStr == _val.m_sshConnectionStr &&
                 m_sshArgs == _val.m_sshArgs &&
                 m_batchMode == _val.m_batchMode &&
                 m_openDomain == _val.m_openDomain );
    }

    bool m_version;
    bool m_connectionString;
    bool m_listWNs;
    bool m_countWNs;
    bool m_status;
    bool m_debug;
    string m_sshConnectionStr;
    string m_sshArgs;
    string m_openDomain;
    bool m_batchMode;
};
//=============================================================================
// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], SOptions *_options ) throw( exception )
{
    if( !_options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", bpo::bool_switch( &( _options->m_version ) ), "Version information." )
    ( "debug,d", bpo::bool_switch( &( _options->m_debug ) ), "Show debug messages." )
    ( "connection_string,c", bpo::bool_switch( &( _options->m_connectionString ) ), "Show PROOF connection string." )
    ( "list,l", bpo::bool_switch( &( _options->m_listWNs ) ), "List all available PROOF workers." )
    ( "number,n", bpo::bool_switch( &( _options->m_countWNs ) ), "Report a number of currently available PROOF workers." )
    ( "status,s", bpo::bool_switch( &( _options->m_status ) ), "Show status of PoD server." )
    ( "ssh", bpo::value<string>(), "An SSH connection string. Directs pod-info to use SSH to detect a remote PoD server." )
    ( "ssh_opt", bpo::value<string>(), "Additinal options, which will be used in SSH commands." )
    ( "ssh_open_domain", bpo::value<string>(), "The name of a third party machine open to the outside world"
                                               " and from which direct connections to the server are possible." )
    ( "batch,b", bpo::bool_switch( &( _options->m_batchMode ) ), "Enable the batch mode." )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    boost_hlp::option_dependency( vm, "ssh_opt", "ssh" );

    if( vm.count( "ssh" ) )
    {
        _options->m_sshConnectionStr = vm["ssh"].as<string>();
    }
    if( vm.count( "ssh_opt" ) )
    {
        _options->m_sshArgs = vm["ssh_opt"].as<string>();
    }
    if( vm.count( "ssh_open_domain" ) )
    {
        _options->m_openDomain = vm["ssh_open_domain"].as<string>();
    }

    // we need an empty struct to check the case when user don't provide any argument
    SOptions s;
    if( vm.count( "help" ) || ( s == *_options ) )
    {
        cout << visible << endl;
        return false;
    }

    return true;
}
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
        return 1;
    }
    killTunnel();
    return 0;
}
