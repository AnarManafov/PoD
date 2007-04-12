/************************************************************************/
/**
 * @file Main.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/cmdline.hpp>

// OUR
#include "PROOFAgent.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
using namespace boost::program_options;

// Function used to check that 'opt1' and 'opt2' are not specified at the same time.
void conflicting_options(const variables_map& vm, const char* opt1, const char* opt2)
{
    if (vm.count(opt1) && !vm[opt1].defaulted() && vm.count(opt2) && !vm[opt2].defaulted())
    {
        string str("Command line parameter \"%1\" conflicts with \"%2\""); // TODO: Move this message to the resource file
        replace<string>( &str, "%1", opt1 );
        replace<string>( &str, "%2", opt2 );
        throw logic_error( str );
    }
}

typedef struct SOptions
{
    typedef enum ECommand { Start, Stop } ECommand_t;

    SOptions(): m_Command(Stop)
    {}

    string m_sConfigFile;
    string m_sInstanceName;
    ECommand_t m_Command;
}
SOptions_t;

// Command line parser
void ParseCmdLine( int _Argc, char *_Argv[], SOptions_t *_Options ) throw (exception)
{
    if ( !_Options )
        throw runtime_error("Internal error: options' container is empty.");

    // Declare the supported options.
    options_description desc("PROOFAgent command line options"); // TODO: Move to the resource file
    desc.add_options()
    ("help,h", "produce help message")
    ("file,f", value<string>(), "configuration file")
    ("start", "start PROOFAgent daemon")
    ("stop", "stop PROOFAgent daemon")
    ("instance,i", value<string>(), "name of the instance of daemon")
    ;
    
    // Parsing command-line
    variables_map vm;
    store(parse_command_line(_Argc, _Argv, desc), vm);
    notify(vm);

    if ( vm.empty() )
        throw logic_error( "PROOFAgent: No command line arguments" );

    conflicting_options(vm, "start", "stop");

    if ( vm.count("help") )
    {
        cout << desc << endl;
        return ;
    }
    if ( vm.count("file") )
        _Options->m_sConfigFile = vm["file"].as<string>();
    if ( vm.count("start") )
        _Options->m_Command = SOptions_t::Start;
    if ( vm.count("stop") )
        _Options->m_Command = SOptions_t::Stop;
    if ( vm.count("instance") )
        _Options->m_sInstanceName = vm["instance"].as<string>();
}

int main( int argc, char *argv[] )
{
    // Command line parser
    SOptions_t Options;
    try
    {
        ParseCmdLine( argc, argv, &Options );
    }
    catch (exception& e)
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return erError;
    }

    // Daemon-specific initialization goes here
    CPROOFAgent agent;
    if ( FAILED( agent.ReadCfg( Options.m_sConfigFile, Options.m_sInstanceName ) ) )
        return erError; // TODO: Log me!


    // process ID and Session ID
    pid_t pid;
    pid_t sid;

    // Fork off the parent process
    pid = fork();
    if ( pid < 0 )
        return ( erError );

    // If we got a good PID, then we can exit the parent process.
    if ( pid > 0 )
        return ( erOK );

    // Change the file mode mask
    umask( 0 );

    // Create a new SID for the child process
    sid = setsid();
    if ( sid < 0 )  // TODO:  Log the failure
        return ( erError );

    // Change the current working directory
    //     chdir("/") to ensure that our process doesn't keep any directory
    //     in use. Failure to do this could make it so that an administrator
    //     couldn't unmount a filesystem, because it was our current directory.
    if ( chdir( "/" ) < 0 ) // TODO: Log the failure
        return ( erError );


    // Close out the standard file descriptors
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );

    // Establish new open descriptors for stdin, stdout, and stderr. Even if
    //  we don't plan to use them, it is still a good idea to have them open.
    int fd = open("/dev/null", O_RDWR); // stdin - file handle 0.
    dup(fd);                        // stdout - file handle 1.
    dup(fd);                        // stderr - file handle 2.

    // Starting Agent
    agent.Start();

    //        // The Big Loop
    //        while ( true )
    //        {
    //            // TODO: Check for "STOP" here
    //            sleep( 30 ); // wait 30 seconds
    //        }

    return erOK;
}
