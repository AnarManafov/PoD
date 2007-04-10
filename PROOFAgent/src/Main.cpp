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
#include "PROOFAgent.h"

using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;

int main( int argc, char *argv[] )
{
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
    if ( chdir( "/" ) < 0 ) // TODO: Log the failure
        return ( erError );

    // Close out the standard file descriptors
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );

    /* Daemon-specific initialization goes here */

    // TODO: Implement application's parameters check
    // "/home/anar/svn/grid/D-Grid/PROOFAgent/trunk/PROOFAgent/documentation/PROOFAgent_config/proofagent.cfg.xml"
    CPROOFAgent agent;
    agent.Init( argv[1] );

//    // The Big Loop
//    while ( true )
//    {
//        // TODO: Check for "STOP" here
//        sleep( 30 ); // wait 30 seconds
//    }

    return erOK;
}
