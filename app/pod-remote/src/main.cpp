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
#include <fstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// MiscCommon
#include "BOOSTHelper.h"
#include "Process.h"
#include "SysHelper.h"
#include "PoDUserDefaultsOptions.h"
#include "INet.h"
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
#include "Options.h"
#include "SSHTunnel.h"
#include "logEngine.h"
#include "Client.h"
#include "Server.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using namespace pod_remote;
using namespace PROOFAgent;
namespace inet = MiscCommon::INet;
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

        if( options.m_executor )
        {
            try
            {
                CServer srv( STDIN_FILENO, STDOUT_FILENO );
                srv.getSrvHostInfo();
            }
            catch( const exception &_e )
            {
                // Server must send its logs to a stdout,
                // so that Client will catch them
                cout << _e.what() << endl;
            }
            return 0;
        }

        // Start the log engine only on clients (local instances)
        slog.start( env.getlogEnginePipeName() );

        // the fork stuff we need only if user wants to send a command
        // to a remote server
        if( !options.m_start && !options.m_stop && !options.m_restart )
            throw runtime_error( "There is nothing to do.\n" );

        if( options.cleanConnectionString().empty() )
            throw runtime_error( "Please provide a connection URL.\n" );
        // Create two pipes, which later will be used for stdout/in redirections
        // TODO: close all handles at the end
        int stdin_pipe[2];
        int stdout_pipe[2];
        int stderr_pipe[2];
        if( -1 == pipe( stdin_pipe ) )
            throw runtime_error( "Error: Can't create a communication stdin_pipe.\n" );

        if( -1 == pipe( stdout_pipe ) )
            throw runtime_error( "Error: Can't create a communication stdout_pipe\n" );

        if( -1 == pipe( stderr_pipe ) )
            throw runtime_error( "Error: Can't create a communication stderr_pipe\n" );

        // fork a child process
        pid_t pid = fork();

        // Make stdout_pipe[0] non-blocking so we can read from it without getting stuck
        fcntl( stdout_pipe[0], F_SETFL, O_NONBLOCK );
        fcntl( stderr_pipe[0], F_SETFL, O_NONBLOCK );

        if( pid == 0 )
        {
            // The child replaces his stdin with stdin_pipe and his stdout with stdout_pipe
            close( stdin_pipe[1] ); // This will never be used by this fork
            close( stdout_pipe[0] ); // This will never be used by this fork
            close( stderr_pipe[0] ); // This will never be used by this fork

            dup2( stdin_pipe[0], STDIN_FILENO );
            dup2( stdout_pipe[1], STDOUT_FILENO );
            dup2( stderr_pipe[1], STDERR_FILENO );
            close( stdin_pipe[0] ); // close one of the file descriptors (stdin  is still open)
            close( stdout_pipe[1] ); // close one of the file descriptors (stdout is still open)
            close( stderr_pipe[1] ); // close one of the file descriptors (stdout is still open)

            // Now, exec this child into a shell process.
            slog( "child: " + options.cleanConnectionString() + '\n' );
            execl( "/usr/bin/ssh", "ssh", "-o StrictHostKeyChecking=no", "-T",
                   options.cleanConnectionString().c_str(), NULL );
            // we must never come to this point
            exit( 1 );
        }
        else
        {
            stringstream sspid;
            sspid << "forking a child process with pid = " << pid << '\n';
            slog( sspid.str() );
            close( stdin_pipe[0] ); // This will never be used by this fork
            close( stdout_pipe[1] ); // This will never be used by this fork
            close( stderr_pipe[1] ); // This will never be used by this fork

            // Write a command to the remote shell
            //
            // source a custom environment script
            if( !options.m_envScript.empty() )
            {
                ifstream f( options.m_envScript.c_str() );
                if( !f.is_open() )
                    throw runtime_error( "can't open: " + options.m_envScript );

                string env(( istreambuf_iterator<char>( f ) ),
                           istreambuf_iterator<char>() );
                slog( "Executing custom environment script...\n" );
                inet::sendall( stdin_pipe[1],
                               reinterpret_cast<const unsigned char*>( env.c_str() ),
                               env.size(), 0 );
            }
            //
            // source PoD environment script
            string pod_env_script( options.remotePoDLocation() );
            if( pod_env_script.empty() )
                throw runtime_error( "Remote PoD location is not specified.\n" );

            smart_append( &pod_env_script, '/' );
            pod_env_script += "PoD_env.sh";
            string pod_env_cmd( "source " );
            pod_env_cmd += pod_env_script;
            // at the end of the string we need to have a new line symbol or a semicolon,
            // since the string will be executed in a shell
            pod_env_cmd += " || exit 1\n";
            inet::sendall( stdin_pipe[1],
                           reinterpret_cast<const unsigned char*>( pod_env_cmd.c_str() ),
                           pod_env_cmd.size(), 0 );
            //
            // start pod-remote on the remote-end
            const char *cmd2 = "pod-server start || exit 1\n";
            inet::sendall( stdin_pipe[1], reinterpret_cast<const unsigned char*>( cmd2 ), strlen( cmd2 ), 0 );

            // start pod-remote on the remote-end
            const char *cmd3 = "pod-info --agentPort && { echo \"OK\"; } || { echo \"NoT OK\"; }; pod-server stop; exit 1\n";
            inet::sendall( stdin_pipe[1], reinterpret_cast<const unsigned char*>( cmd3 ), strlen( cmd3 ), 0 );

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

            //   CClient cl( stdout_pipe[0], stdin_pipe[1] );
            //   SHostInfoCmd srvHostInfo;
            //   cl.getSrvHostInfo( &srvHostInfo );
        }

        // start the monitoring
        //boost::thread monitorThread( boost::bind( monitor, pid, pipe2[1] ) );

        // Main select loop
        string remote_output;
        while( true )
        {
            fd_set readset;
            FD_ZERO( &readset );
            FD_SET( stdout_pipe[0], &readset );
            FD_SET( stderr_pipe[0], &readset );
            int fd_max = stdout_pipe[0] > stderr_pipe[0] ? stdout_pipe[0] : stderr_pipe[0];
            int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );

            if( EBADF == errno )
                break;

            if( retval < 0 )
            {
                cerr << "Problem in the log engine: " << errno2str() << endl;
                break;
            }

            int fdToRead = 0;
            if( FD_ISSET( stdout_pipe[0], &readset ) )
                fdToRead = stdout_pipe[0];
            if( FD_ISSET( stderr_pipe[0], &readset ) )
                fdToRead = stderr_pipe[0];

            if( fdToRead > 0 )
            {
                const int read_size = 64;
                char buf[read_size];
                while( true )
                {
                    int numread = read( fdToRead, buf, read_size );
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
        string msg( e.what() );
        msg += '\n';
        slog( msg );
        return 1;
    }
    return 0;
}
