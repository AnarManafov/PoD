/************************************************************************/
/**
 * @file Main.cpp
 * @brief Implementation of the "Main" function
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// API
#include <sys/wait.h>
// Our
#include "version.h"
#include "Process.h"
#include "PROOFAgent.h"
#include "PARes.h"
#include "BOOSTHelper.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;


void PrintVersion()
{
    // TODO: make VERSION to be taken from the build
    cout
    << PROJECT_NAME << " " << PROJECT_VERSION_STRING << "\n"
    << "protocol version: " << CProtocol::version() << "\n"
    << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}

// Command line parser
bool ParseCmdLine( int _Argc, char *_Argv[], SOptions_t *_Options ) throw( exception )
{
    if ( !_Options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description options( "pod-agent options" );
    options.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "config,c", bpo::value<string>(), "A PoD configuration file" )
    ( "mode,m", bpo::value<string>()->default_value( "server" ), "Agent's mode (use: server or worker)" )
    ( "daemonize,d", "Run agent as a daemon" )
    ( "start", "Start agent daemon (default action)" )
    ( "stop", "Stop agent daemon" )
    ( "status", "Query current status of agent daemon" )
    ( "pidfile,p", bpo::value<string>()->default_value( "/tmp/" ), "Directory, where daemon can keep its pid file" ) // TODO: I am thinking to move this option to config file
    ( "serverinfo", bpo::value<string>()->default_value("$POD_LOCATION/etc/server_info.cfg"), "A server info file name" )
    ( "proofport", bpo::value<unsigned int>(), "A PROOF (xproof) port. Used only by agents in a worker mode" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( options ).run(), vm );

    bpo::notify( vm );

    if ( vm.count( "help" ) || vm.empty() )
    {
        cout << options << endl;
        return false;
    }
    if ( vm.count( "version" ) )
    {
        PrintVersion();
        return false;
    }

    if ( !vm.count( "config" ) )
    {
        cout << options << endl;
        throw runtime_error( "You need to specify a PoD configuration file at least." );
    }

    {
        PoD::CPoDUserDefaults user_defaults;
        user_defaults.init( vm["config"].as<string>() );
        _Options->m_podOptions = user_defaults.getOptions();
    }

    boost_hlp::conflicting_options( vm, "start", "stop" );
    boost_hlp::conflicting_options( vm, "start", "status" );
    boost_hlp::conflicting_options( vm, "stop", "status" );
    boost_hlp::option_dependency( vm, "start", "daemonize" );
    boost_hlp::option_dependency( vm, "stop", "daemonize" );
    boost_hlp::option_dependency( vm, "status", "daemonize" );

    if ( vm.count("mode") )
    {
        string mode( vm["mode"].as<string>() );
        if ( "worker" == mode )
            _Options->m_agentMode = Client;
    }

    if ( vm.count( "start" ) )
        _Options->m_Command = SOptions_t::Start;
    if ( vm.count( "stop" ) )
        _Options->m_Command = SOptions_t::Stop;
    if ( vm.count( "status" ) )
        _Options->m_Command = SOptions_t::Status;
    if ( vm.count( "pidfile" ) )
    {
        _Options->m_sPidfileDir = vm["pidfile"].as<string>();
        // We need to be sure that there is "/" always at the end of the path
        smart_append( &_Options->m_sPidfileDir, '/' );
    }
    _Options->m_bDaemonize = vm.count( "daemonize" );

    if ( vm.count("serverinfo") )
    {
        _Options->m_serverInfoFile = vm["serverinfo"].as<string>();
        smart_path( &_Options->m_serverInfoFile );
    }

    if ( vm.count("proofport") )
        _Options->m_proofPort = vm["proofport"].as<unsigned int>();

    return true;
}

int main( int argc, char *argv[] )
{
    // Command line parser
    SOptions_t Options;
    try
    {
        if ( !ParseCmdLine( argc, argv, &Options ) )
            return erOK;
    }
    catch ( exception& e )
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return erError;
    }

    // Normalizing paths of common options
    PoD::SCommonOptions_t &common = (Server == Options.m_agentMode)?
                                    Options.m_podOptions.m_server.m_common:
                                    Options.m_podOptions.m_worker.m_common;
    smart_path( &common.m_proofCFG );
    // resolving user's home dir from (~/ or $HOME, if present)
    smart_path( &common.m_workDir );
    // We need to be sure that there is "/" always at the end of the path
    smart_append<string>( &common.m_workDir, '/' );
    smart_path( &common.m_logFileDir );
    smart_append<string>( &common.m_logFileDir, '/' );


    // pidfile name: proofagent.<instance_name>.pid
    stringstream pidfile_name;
    pidfile_name
    << Options.m_sPidfileDir
    << "proofagent.pid";

    // Checking for "status" option
    if ( Options.m_Command == SOptions_t::Status )
    {
        pid_t pid = CPIDFile::GetPIDFromFile( pidfile_name.str() );
        if ( pid > 0 && IsProcessExist( pid ) )
        {
            cout << "PROOFAgent process ("
            << pid
            << ") is running..." << endl;
        }
        else
        {
            cout << "PROOFAgent is not running..." << endl;
        }

        return erOK;
    }

    // Checking for "stop" option
    if ( SOptions_t::Stop == Options.m_Command )
    {
        // TODO: make wait for the process here to check for errors
        const pid_t pid_to_kill = CPIDFile::GetPIDFromFile( pidfile_name.str() );
        if ( pid_to_kill > 0 && IsProcessExist( pid_to_kill ) )
        {
            cout
            << "PROOFAgent: closing PROOFAgent ("
            << pid_to_kill
            << ")..." << endl;
            // TODO: Maybe we need more validations of the process before send a signal. We don't want to kill someone else.
            kill( pid_to_kill, SIGTERM );

            // Waiting for the process to finish
            size_t iter( 0 );
            const size_t max_iter = 30;
            while ( iter <= max_iter )
            {
                if ( !IsProcessExist( pid_to_kill ) )
                {
                    cout << endl;
                    break;
                }
                cout << ".";
                cout.flush();
                sleep( 2 ); // sleeping 2 seconds
                ++iter;
            }
            if ( IsProcessExist( pid_to_kill ) )
                cerr << "FAILED! PROOFAgent has failed to close the process." << endl;
        }

        return erOK;
    }

    if ( Options.m_bDaemonize )
    {
        // process ID and Session ID
        pid_t pid;
        pid_t sid;

        // Fork off the parent process
        pid = ::fork();
        if ( pid < 0 )
            return ( erError );

        // If we got a good PID, then we can exit the parent process.
        if ( pid > 0 )
            return ( erOK );

        // Change the file mode mask
        ::umask( 0 );

        // Create a new SID for the child process
        sid = ::setsid();
        if ( sid < 0 )  // TODO:  Log the failure
            return ( erError );
    }

    // Main object - agent itself
    CPROOFAgent agent;

    try
    {
        CPIDFile pidfile( pidfile_name.str(), ( Options.m_bDaemonize ) ? ::getpid() : 0 );

        // Daemon-specific initialization goes here
        agent.setConfiguration( Options );

        if ( Options.m_bDaemonize )
        {
            // Change the current working directory
            //     chdir("/") to ensure that our process doesn't keep any directory
            //     in use. Failure to do this could make it so that an administrator
            //     couldn't unmount a file system, because it was our current directory.
            if ( ::chdir( "/" ) < 0 ) // TODO: Log the failure
                return ( erError );

            // Close out the standard file descriptors
            close( STDIN_FILENO );
            close( STDOUT_FILENO );
            close( STDERR_FILENO );

            // Establish new open descriptors for stdin, stdout, and stderr. Even if
            //  we don't plan to use them, it is still a good idea to have them open.
            int fd = open( "/dev/null", O_RDWR ); // stdin - file handle 0.
            dup( fd );                      // stdout - file handle 1.
            dup( fd );                      // stderr - file handle 2.
        }

        // Starting Agent
        agent.Start();
    }
    catch ( exception &e )
    {
        agent.FaultLog( erError, e.what() );
        return 1;
    }
    catch ( ... )
    {
        string errMsg( "Unexpected Exception occurred." );
        agent.FaultLog( erXMLInit, errMsg );
        return 1;
    }

    return 0;
}
