/************************************************************************/
/**
 * @file Main.cpp
 * @brief Implementation of the "Main" function 
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "config.h"

// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/cmdline.hpp>

// API
#include <sys/wait.h>

// OUR
#include "Process.h"
#include "PROOFAgent.h"
#include "PARes.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
using namespace boost::program_options;

// Function used to check that 'opt1' and 'opt2' are not specified at the same time.
void conflicting_options( const variables_map& _vm, const char* _opt1, const char* _opt2 )
{
    if ( _vm.count(_opt1) && !_vm[_opt1].defaulted() && _vm.count(_opt2) && !_vm[_opt2].defaulted() )
    {
        string str("Command line parameter \"%1\" conflicts with \"%2\""); // TODO: Move this message to the resource file
        replace<string>( &str, "%1", _opt1 );
        replace<string>( &str, "%2", _opt2 );
        throw runtime_error( str );
    }
}
// Function used to check that of 'for_what' is specified, then 'required_option' is specified too.
void option_dependency(const variables_map &_vm, const char *_for_what, const char *_required_option)
{
    if ( _vm.count(_for_what) && !_vm[_for_what].defaulted() && ( !_vm.count(_required_option) || _vm[_required_option].defaulted() ) )
    {
        string str("Command line parameter \"%1\" must be used with \"%2\""); // TODO: Move this message to the resource file
        replace<string>( &str, "%1", _for_what );
        replace<string>( &str, "%2", _required_option );
        throw runtime_error( str );
    }
}

// PROOFAgent's container of options
typedef struct SOptions
{
    typedef enum ECommand { Start, Stop, Status } ECommand_t;

    SOptions():                        // Default options' values
            m_Command(Start),
            m_sPidfileDir("/tmp/"),
            m_bDaemonize(false)
    {}

    string m_sConfigFile;
    string m_sInstanceName;
    ECommand_t m_Command;
    string m_sPidfileDir;
    bool m_bDaemonize;
}
SOptions_t;

void PrintVersion()
{
    cout
    << "PROOFAgent v." << VERSION << "\n"
    << "-------------------------------------------------" << "\n"
    << "application file name: " << PACKAGE << "\n"
    << "protocol version: " << g_szPROTOCOL_VERSION << "\n"
    << "-------------------------------------------------" << "\n"
    << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}

// Command line parser
bool ParseCmdLine( int _Argc, char *_Argv[], SOptions_t *_Options ) throw (exception)
{
    if ( !_Options )
        throw runtime_error("Internal error: options' container is empty.");

    // Declare the supported options.
    options_description desc("PROOFAgent command line options"); // TODO: Move to the resource file
    desc.add_options()
    ("help,h", "produce help message")
    ("config,c", value<string>(), "configuration file")
    ("start", "start PROOFAgent daemon (default action)")
    ("stop", "stop PROOFAgent daemon")
    ("status", "query current status of PROOFAgent daemon")
    ("instance,i", value<string>(), "name of the instance of PROOFAgent")
    ("pidfile,p", value<string>(), "directory where daemon can keep its pid file. (Default: /tmp/)") // TODO: I am thinking to move this option to config file
    ("daemonize,d", "run PROOFAgent as a daemon")
    ("version,v", "Version information")
    ;

    // Parsing command-line
    variables_map vm;
    store(parse_command_line(_Argc, _Argv, desc), vm);
    notify(vm);

    if ( vm.count("help") || vm.empty() )
    {
        cout << desc << endl;
        return false;
    }

    if ( vm.count("version") )
    {
        PrintVersion();
        return false;
    }

    if ( !vm.count("instance") )
        throw runtime_error("You need to specify instance name by using \"instance\" option.");

    conflicting_options( vm, "start", "stop" );
    conflicting_options( vm, "start", "status" );
    conflicting_options( vm, "stop", "status" );
    option_dependency(vm, "start", "daemonize" );
    option_dependency(vm, "stop", "daemonize" );
    option_dependency(vm, "status", "daemonize" );

    if ( vm.count("config") )
        _Options->m_sConfigFile = vm["config"].as<string>();
    if ( vm.count("start") )
        _Options->m_Command = SOptions_t::Start;
    if ( vm.count("stop") )
        _Options->m_Command = SOptions_t::Stop;
    if ( vm.count("status") )
        _Options->m_Command = SOptions_t::Status;
    if ( vm.count("instance") )
        _Options->m_sInstanceName = vm["instance"].as<string>();
    if ( vm.count("pidfile") )
    {
        _Options->m_sPidfileDir = vm["pidfile"].as<string>();
        // We need to be sure that there is "/" always at the end of the path
        smart_append( &_Options->m_sPidfileDir, '/' );
    }
    _Options->m_bDaemonize = vm.count("daemonize");

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
    catch (exception& e)
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
        if ( pid > 0 && IsProcessExist(pid) )
        {
            cout << "PROOFAgent process ("
            << pid
            << ") is running..." << endl;
        }
        else
        {
            cout << "PROOFAgent is not running..." << endl;
            ;
        }

        return erOK;
    }

    // Checking for "stop" option
    if ( Options.m_Command == SOptions_t::Stop )
    {
        // TODO: make wait for the process here to check for errors
        pid_t pid_to_kill = CPIDFile::GetPIDFromFile( pidfile_name.str() );
        if ( pid_to_kill > 0 && IsProcessExist(pid_to_kill) )
        {
            cout
            << "PROOFAgent: closing PROOFAgent ("
            << pid_to_kill
            << ")..." << endl;
            ::kill( pid_to_kill, SIGTERM ); // TODO: Maybe we need more validations of the process before send a signal. We don't want to kill someone else.

            // Waiting for the process to finish
            size_t iter(0);
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
                sleep(2); // sleeping 2 seconds
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
        CPIDFile pidfile( pidfile_name.str(), (Options.m_bDaemonize) ? ::getpid() : 0 );

        // Daemon-specific initialization goes here
        if ( FAILED( agent.ReadCfg( Options.m_sConfigFile, Options.m_sInstanceName ) ) )
            return erError; // TODO: Log me!

        int fd;
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
            fd = open("/dev/null", O_RDWR); // stdin - file handle 0.
            dup(fd);                        // stdout - file handle 1.
            dup(fd);                        // stderr - file handle 2.
        }

        // Starting Agent
        agent.Start();
    }
    catch ( exception &e )
    {
        agent.FaultLog( erError, e.what() );
    }

    return erOK;
}
