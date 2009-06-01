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
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/cmdline.hpp>
//#include <boost/tokenizer.hpp>
//#include <boost/token_functions.hpp>
// API
#include <sys/wait.h>
// OUR
#include "Process.h"
#include "PROOFAgent.h"
#include "PARes.h"
#include "BOOSTHelper.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
using namespace boost;
using namespace boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;


void PrintVersion()
{
    // TODO: make VERSION to be taken from the build
    cout
    << "PROOFAgent v." << "2.0.0" << "\n"
    << "application file name: " << "proofagent" << "\n"
    << "protocol version: " << g_szPROTOCOL_VERSION << "\n"
    << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}

// Command line parser
bool ParseCmdLine( int _Argc, char *_Argv[], SOptions_t *_Options ) throw( exception )
{
    if ( !_Options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    options_description generic( "Generic options" );
    generic.add_options()
    ( "help,h", "produce help message" )
    ( "version,v", "Version information" )
    ( "config,c", value<string>(), "configuration file" )
    ( "daemonize,d", "run PROOFAgent as a daemon" )
    ( "start", "start PROOFAgent daemon (default action)" )
    ( "stop", "stop PROOFAgent daemon" )
    ( "status", "query current status of PROOFAgent daemon" )
    ( "pidfile,p", value<string>()->default_value( "/tmp/" ), "directory where daemon can keep its pid file." ) // TODO: I am thinking to move this option to config file
    ;

    options_description config_file_options( "Configuration" );
    config_file_options.add_options()
    ( "general.isServerMode", value<bool>( &_Options->m_GeneralData.m_isServerMode )->default_value( true, "yes" ), "todo: desc" )
    ( "general.work_dir", value<string>( &_Options->m_GeneralData.m_sWorkDir )->default_value( "$POD_LOCATION/" ), "" )
    ( "general.logfile_dir", value<string>( &_Options->m_GeneralData.m_sLogFileDir )->default_value( "$POD_LOCATION/log" ), "" )
    ( "general.logfile_overwrite", value<bool>( &_Options->m_GeneralData.m_bLogFileOverwrite )->default_value( false, "no" ), "" )
    ( "general.log_level", value<size_t>( &_Options->m_GeneralData.m_logLevel )->default_value( 0 ), "" )
    ( "general.timeout", value<size_t>( &_Options->m_GeneralData.m_nTimeout )->default_value( 0 ), "" )
    ( "general.proof_cfg_path", value<string>( &_Options->m_GeneralData.m_sPROOFCfg )->default_value( "~/proof.conf" ), "" )
    ( "general.last_execute_cmd", value<string>( &_Options->m_GeneralData.m_sLastExecCmd ), "" )

    ( "server.listen_port", value<unsigned short>( &_Options->m_serverData.m_nPort )->default_value( 22001 ), "" )
    ( "server.local_client_port_min", value<unsigned short>( &_Options->m_serverData.m_nLocalClientPortMin )->default_value( 20000 ), "" )
    ( "server.local_client_port_max", value<unsigned short>( &_Options->m_serverData.m_nLocalClientPortMax )->default_value( 25000 ), "" )

    ( "client.server_port", value<unsigned short>( &_Options->m_clientData.m_nServerPort )->default_value( 22001 ), "" )
    ( "client.server_addr", value<string>( &_Options->m_clientData.m_strServerHost )->default_value( "lxi020.gsi.de" ), "" )
    ( "client.local_proofd_port", value<unsigned short>( &_Options->m_clientData.m_nLocalClientPort )->default_value( 111 ), "" )
    ;

    options_description visible( "PROOFAgent options" );
    visible.add( generic ).add( config_file_options );

    // Parsing command-line
    variables_map vm;
    //store(parse_command_line(_Argc, _Argv, desc), vm);
    store( command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );

    notify( vm );

    if ( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if ( vm.count( "version" ) )
    {
        PrintVersion();
        return false;
    }
    if ( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }
    else
    {
        // Load the file and tokenize it
        ifstream ifs( vm["config"].as<string>().c_str() );
        if ( !ifs.good() )
        {
            cout << "Could not open the configuration file" << endl;
            return 1;
        }
        // Parse the config file
        store( parse_config_file( ifs, config_file_options ), vm );
        notify( vm );
    }

    boost_hlp::conflicting_options( vm, "start", "stop" );
    boost_hlp::conflicting_options( vm, "start", "status" );
    boost_hlp::conflicting_options( vm, "stop", "status" );
    boost_hlp::option_dependency( vm, "start", "daemonize" );
    boost_hlp::option_dependency( vm, "stop", "daemonize" );
    boost_hlp::option_dependency( vm, "status", "daemonize" );

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

    // pidfile name: proofagent.<instance_name>.pid
    stringstream pidfile_name;
    pidfile_name
    << Options.m_sPidfileDir
    << "proofagent."
    << Options.m_sInstanceName
    << ".pid";

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
            ::kill( pid_to_kill, SIGTERM ); // TODO: Maybe we need more validations of the process before send a signal. We don't want to kill someone else.

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
        agent.setConfiguration( &Options );

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
