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
// MiscCommon

//HACK: Keep BOOSTHelper.h too silence the warning:
// /opt/local/include/boost/thread/pthread/condition_variable.hpp: In member function ‘void boost::condition_variable::wait(boost::unique_lock<boost::mutex>&)’:
// /opt/local/include/boost/thread/pthread/condition_variable.hpp:53: warning: unused variable ‘res’
// Remove it as soon as BOOST is fixed
#include "BOOSTHelper.h"
#include "Process.h"
#include "SysHelper.h"
#include "INet.h"
#include "logEngine.h"
#include "SSHTunnel.h"
// pod-remote
#include "version.h"
#include "Environment.h"
#include "Options.h"
#include "MessageParser.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using namespace pod_remote;
namespace inet = MiscCommon::INet;
//=============================================================================
void version()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
void send_cmd( int _handle, const string &_cmd, bool _stopSrvIfFailed = true )
{
    stringstream cmd;
    cmd << _cmd
        << " && { echo \"" << g_message_OK << "\"; } || ";

    if( _stopSrvIfFailed )
        cmd << "{ pod-server stop; exit 1; }; \n";
    else
        cmd << "{ exit 1; }; \n";

    inet::sendall( _handle,
                   reinterpret_cast<const unsigned char*>( cmd.str().c_str() ),
                   cmd.str().size(), 0 );
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

    pid_t pid( 0 );
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];
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

        // Do we need to use the prevues server if there was any...
        if( does_file_exists( env.remoteCfgFile() ) && options.m_sshConnectionStr.empty() )
        {
            SPoDRemoteOptions opt_file;
            opt_file.load( env.remoteCfgFile() );
            options.m_sshConnectionStr = opt_file.m_connectionString;
            options.m_sshConnectionStr += ':';
            options.m_sshConnectionStr += opt_file.m_PoDLocation;
            options.m_envScript = opt_file.m_env;
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
        if( -1 == pipe( stdin_pipe ) )
            throw runtime_error( "Error: Can't create a communication stdin_pipe.\n" );

        if( -1 == pipe( stdout_pipe ) )
            throw runtime_error( "Error: Can't create a communication stdout_pipe\n" );

        if( -1 == pipe( stderr_pipe ) )
            throw runtime_error( "Error: Can't create a communication stderr_pipe\n" );

        // fork a child process
        pid = fork();

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

            slog( "Executing PoD environment script...\n" );
            //
            // source PoD environment script
            string pod_env_script( options.remotePoDLocation() );
            if( pod_env_script.empty() )
                throw runtime_error( "Remote PoD location is not specified.\n" );

            smart_append( &pod_env_script, '/' );
            pod_env_script += "PoD_env.sh";
            string pod_env_cmd( "source " );
            pod_env_cmd += pod_env_script;
            send_cmd( stdin_pipe[1], pod_env_cmd, false );
            // drain the stdout in the pipes
            SMessageParserOK msg_ok;
            CMessageParser<SMessageParserOK, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
            msg.parse( msg_ok, slog );
        }

        if( options.m_start || options.m_restart )
        {
            // We allow so far to start only one remote PoD server at time.
            //
            // The following is to be done on the remote machine:
            //
            // 1. Start a remote pod-server
            //
            // TODO: user needs to have a ROOT env. initialized  (for example in PoD_env.sh)
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
            // 5. Create a config file, where information about the remote server
            // will be collected. This file late will be used by other commands.
            //
            stringstream ss;
            ss
                    << "Starting the remote PoD server on "
                    << options.m_sshConnectionStr << '\n';
            slog( ss.str() );

            size_t agentPort( 0 );
            size_t xpdPort( 0 );
            if( options.m_restart )
            {
                send_cmd( stdin_pipe[1], "pod-server restart" );
                SMessageParserString msg_string;
                CMessageParser<SMessageParserString, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_string, slog );
                slog( msg_string.get() );
            }
            else
            {
                send_cmd( stdin_pipe[1], "pod-server start" );

                SMessageParserOK msg_ok;
                CMessageParser<SMessageParserOK, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_ok, slog );
            }
            slog( "Remote PoD server is strated\n" );
            slog( "Checking service ports...\n" );
            // check for pod-agent port on the remote server
            {
                send_cmd( stdin_pipe[1], "pod-info --agentPort" );

                SMessageParserNumber msg_num;
                CMessageParser<SMessageParserNumber, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_num, slog );
                agentPort = msg_num.get();
            }
            // check for xproofd port on the remote server
            {
                send_cmd( stdin_pipe[1], "pod-info --xpdPort" );

                SMessageParserNumber msg_num;
                CMessageParser<SMessageParserNumber, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_num, slog );
                xpdPort = msg_num.get();
            }
            if( 0 == agentPort || 0 == xpdPort )
                throw runtime_error( "Can't detect remote ports."
                                     " Please try to start PoD server again." );

            // Start SSH tunnel

            // find a free port to listen on
            size_t agentPortListen = inet::get_free_port( env.getUD().m_server.m_agentPortsRangeMin,
                                                          env.getUD().m_server.m_agentPortsRangeMax );
            size_t xpdPortListen = inet::get_free_port( env.getUD().m_server.m_common.m_xproofPortsRangeMin,
                                                        env.getUD().m_server.m_common.m_xproofPortsRangeMax );

            if( 0 == agentPortListen || 0 == xpdPortListen )
            {
                throw runtime_error( "Can't find any free port to tunnel PoD services" );
            }

            CSSHTunnel sshTunnelAgent;
            sshTunnelAgent.deattach();
            sshTunnelAgent.setPidFile( env.getTunnelPidFileAgent() );
            sshTunnelAgent.create( options.cleanConnectionString(), agentPortListen,
                                   agentPort, options.m_openDomain );

            CSSHTunnel sshTunnelXpd;
            sshTunnelXpd.deattach();
            sshTunnelXpd.setPidFile( env.getTunnelPidFileXpd() );
            sshTunnelXpd.create( options.cleanConnectionString(), xpdPortListen,
                                 xpdPort, options.m_openDomain );

            // Store information about the new remote server
            // 1. write server_info.cfg, so that local services can connect to it
            // 2. write remote_server_info.cfg for other pod-remote calls
            SPoDRemoteOptions opt_file;
            opt_file.m_connectionString = options.cleanConnectionString();
            opt_file.m_PoDLocation = options.remotePoDLocation();
            opt_file.m_env = options.m_envScript;
            opt_file.m_localAgentPort = agentPortListen;
            opt_file.m_localXpdPort = xpdPortListen;
            opt_file.save( env.remoteCfgFile() );
        }
        else if( options.m_stop )
        {
            stringstream ss;
            ss
                    << "Stopping the remote PoD server on "
                    << options.m_sshConnectionStr << '\n';
            slog( ss.str() );

            {
                send_cmd( stdin_pipe[1], "pod-server stop" );

                SMessageParserOK msg_ok;
                CMessageParser<SMessageParserOK, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_ok, slog );
            }

            slog( "Clossing SSH tunnels...\n" );
            CSSHTunnel sshTunnelAgent;
            sshTunnelAgent.setPidFile( env.getTunnelPidFileAgent() );
            CSSHTunnel sshTunnelXpd;
            sshTunnelXpd.setPidFile( env.getTunnelPidFileXpd() );
        }
        // close the SSH terminal
        if( pid > 0 )
            kill( pid, SIGKILL );
    }
    catch( exception& e )
    {
        // close the SSH terminal
        if( pid > 0 )
            kill( pid, SIGKILL );

        string msg( e.what() );
        msg += '\n';
        slog( msg );
        return 1;
    }
    return 0;
}
