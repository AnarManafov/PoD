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
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
#include "Options.h"
#include "SrvInfo.h"
#include "SSHTunnel.h"
#include "logEngine.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
void version()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
// The main idea with this method is to create a couple of pipes
// that can be used to redirect the stdin and stdout of the
// remote shell process (telnet, rsh, ssh, ...whatever) to this
// program so they can be read and written to by read() and write().
//
// This is accomplished by first setting creating the pipe, then
// forking this process into 2 processes, then having the "child"
// fork replace his stdin, stdout pipes with the newly created ones,
// and finally having the child "exec" itself into the remote
// shell program. Since the remote shell will inherit its stdin
// and stdout streams from the fork, it will use the pipes we setup
// for that purpose.
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/errno.h>
//#include <time.h>
//
//#define _DBG_ cout<<__FILE__<<":"<<__LINE__<<" "
//#define _DBG__ _DBG_<<endl;
//
//int main(int narg, char *argv[])
//{
//    //char *const args[]={"ls",NULL};
//    //execv("/bin/sh", args);
//    //return 0;
//    
//    // Create 2 pipes
//    int fdA[2];
//    int fdB[2];
//    pipe(fdA);
//    pipe(fdB);
//    
//    // The following splits us into 2 identical processes except there
//    // is a parent child relationship between the two. The "child" will
//    // have pid==0 while the parent will not.
//    pid_t pid = fork();
//    
//    // Make fdB[0] non-blocking so we can read from it without getting stuck
//    fcntl(fdB[0], F_SETFL, O_NONBLOCK);
//    
//    if(pid==0){
//        // The child replaces his stdin with fdA and his stdout with fdB
//        close(fdA[1]); // This will never be used by this fork
//        close(fdB[0]); // This will never be used by this fork
//        
//        dup2(fdA[0], STDIN_FILENO);
//        dup2(fdB[1], STDOUT_FILENO);
//        close(fdA[0]); // close one of the file descriptors (stdin  is still open)
//        close(fdB[1]); // close one of the file descriptors (stdout is still open)
//        
//        
//        // Now, exec this fork into a shell process. The first argument of execl
//        // is the actual program to run. The second argument starts the "argv"
//        // list passed into the program which usually contains the name of the
//        // program as invoked in the shell as the 0-th argument.
//        char *const args[]={NULL};
//        execl("/usr/bin/ssh", "ssh", "-T", "manafov@lxg0527", 0);
//    }else{
//        close(fdA[0]); // This will never be used by this fork
//        close(fdB[1]); // This will never be used by this fork
//        
//        // Write a command to the remote shell
//        const char *cmd = ". rootlogin 527-06b-xrd; source pod_setup new\n";
//        _DBG_<<"write():"<<write(fdA[1], cmd, strlen(cmd))<<endl;
//        
//        const char *cmd2 = "pod-server start\n";
//        _DBG_<<"write():"<<write(fdA[1], cmd2, strlen(cmd2))<<endl;
//        
//        // Read response. For this example, we keep reading for at least
//        // 10 seconds before quitting. Draw a line of "=" signs for each
//        // successful read to indicate where the system breaks it up
//        // (I think always ar newlines).
//        time_t start_time = ti	me(NULL);
//        do{
//            char buff[8192];
//            bzero(buff, 8192);
//            int rv = read(fdB[0], buff, 8192);
//            if(rv>0){
//                // cout<<"====================================================="<<endl;
//                cout<<buff;
//            }
//        }while((time(NULL)-start_time) < 10);
//    }
//    
//    return 0;
//}
int main( int argc, char *argv[] )
{
    CLogEngine slog;
    CEnvironment env;

    try
    {
        SOptions options;
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        // Show version information
        if( options.m_version )
        {
            version();
            return 0;
        }

        env.init();
        slog.start( env.getlogEnginePipeName() );

        if( options.m_start )
        {
            // We allow so far to start only one remote PoD server at time.
            //
            // The following is to be done on the remote machine:
            //
            // 1. Start a remote pod-server
            //
            // TODO: user needs to have a ROOT env. initialized (for example in PoD_env.sh)
            // or we have to give this possibility via a custom env script...
            // xproofd and ROOT libs must be in the $PATH
            //
            // 2. Check the ports for proof and pod-agent
            //
            // 3. Create SSH tunnels on proof and pod-agent ports
            //
            // 4. Configure the local env. to work with the remote server,
            // pod-info must not see the difference.
            //
            stringstream ss;
            ss
                    << "Starting a remote PoD server on "
                    << options.m_sshConnectionStr << '\n';
            slog( ss.str() );

        }
    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }
    return 0;
}
