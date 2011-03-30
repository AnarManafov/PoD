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
void monitor( pid_t _pid, int _pipe_fd )
{
//    cout << "Monitor thread" << endl;
//    //if (!IsProcessExist(_pid))
//    int stat;
//    if( _pid == ::waitpid( _pid, &stat, WNOHANG ) )
//    {
//        cout << "DEBUG!!!!!!" << endl;
//        const char *cmd = "exit\n";
//        write( _pipe_fd, cmd, strlen( cmd ) );
//        return;
//    }
//    sleep( 2 );
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

        // Create two pipes, which later will be used for stdout/in redirections
        int pipe1[2];
        int pipe2[2];
        pipe( pipe1 );
        pipe( pipe2 );

        // fork a child process
        pid_t pid = fork();

        // Make pipe2[0] non-blocking so we can read from it without getting stuck
        fcntl( pipe2[0], F_SETFL, O_NONBLOCK );

        if( pid == 0 )
        {
            // The child replaces his stdin with pipe1 and his stdout with fdB
            close( pipe1[1] ); // This will never be used by this fork
            close( pipe2[0] ); // This will never be used by this fork

            dup2( pipe1[0], STDIN_FILENO );
            dup2( pipe2[1], STDOUT_FILENO );
            close( pipe1[0] ); // close one of the file descriptors (stdin  is still open)
            close( pipe2[1] ); // close one of the file descriptors (stdout is still open)


            // Now, exec this child into a shell process.
            slog( "child: " + options.cleanConnectionString() + '\n' );
            execl( "/usr/bin/ssh", "ssh", "-T", options.cleanConnectionString().c_str(), NULL );
            // we must never come to this point
            exit( 1 );
        }
        else
        {
            stringstream sspid;
            sspid << "forking a child process pid = " << pid << '\n';
            slog( sspid.str() );
            close( pipe1[0] ); // This will never be used by this fork
            close( pipe2[1] ); // This will never be used by this fork

            // Write a command to the remote shell
            const char *cmd = ". rootlogin 527-06b-xrd; source pod_setup new\n";
            write( pipe1[1], cmd, strlen( cmd ) );

            const char *cmd2 = "exit\n";
            write( pipe1[1], cmd2, strlen( cmd2 ) );
        }

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

        // start the monitoring
        //boost::thread monitorThread( boost::bind( monitor, pid, pipe2[1] ) );

        // Main select loop
        string remote_output;
        while( true )
        {
            fd_set readset;
            FD_ZERO( &readset );
            FD_SET( pipe2[0], &readset );
            int retval = ::select( pipe2[0] + 1, &readset, NULL, NULL, NULL );

            if( EBADF == errno )
                break;

            if( retval < 0 )
            {
                cerr << "Problem in the log engine: " << errno2str() << endl;
                break;
            }

            if( FD_ISSET( pipe2[0], &readset ) )
            {
                const int read_size = 64;
                char buf[read_size];
                while( true )
                {
                    int numread = read( pipe2[0], buf, read_size );
                    if( 0 == numread )
                        return 0;

                    if( numread > 0 )
                    {
                        for( int i = 0; i < numread; ++i )
                        {
                            remote_output += buf[i];
                            if( '\n' == buf[i] )
                            {
                                slog( "remote end reports: " + remote_output );
                                remote_output.clear();
                            }
                        }
                    }
                    else
                        break;
                }
                cout.flush();
            }
        }

    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }
    return 0;
}
