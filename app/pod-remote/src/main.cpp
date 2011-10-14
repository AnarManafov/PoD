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
// /opt/local/include/boost/thread/pthread/condition_variable.hpp:
//    In member function ‘void boost::condition_variable::wait(boost::unique_lock<boost::mutex>&)’:
// /opt/local/include/boost/thread/pthread/condition_variable.hpp:53: warning: unused variable ‘res’
// Remove it as soon as BOOST is fixed
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "INet.h"
#include "logEngine.h"
#include "SSHTunnel.h"
#include "PoDUserDefaultsOptions.h"
// pod-remote
#include "version.h"
#include "PoDSysFiles.h"
#include "Options.h"
#include "MessageParser.h"
#include "Utils.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
using namespace pod_remote;
namespace inet = MiscCommon::INet;
//=============================================================================
sig_atomic_t graceful_quit = 0;
//=============================================================================
void signal_handler( int _SignalNumber )
{
    graceful_quit = 1;
}
//=============================================================================
void signal_handler_hungup( int _SignalNumber )
{
    cerr << PROJECT_NAME << " fatal: The remote end hung up unexpectedly" << endl;
}
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

    inet::writeall( _handle, cmd.str() );
}
//=============================================================================
void monitor( int _fdIn, int _fdOut, int _fdErr )
{
    try
    {
        CLogEngine NullLog;
        while( true )
        {
            send_cmd( _fdIn, "pod-server status_with_code", false );

            SMessageParserOK msg_ok;
            CMessageParser<SMessageParserOK, CLogEngine> msg( _fdOut, _fdErr );
            msg.parse( msg_ok, NullLog );
            if( !msg_ok.get() || graceful_quit )
            {
                if( msg_ok.get() )
                    send_cmd( _fdIn, "pod-server stop" );

                return;
            }
            // Check every 30 seconds
            sleep( 30 );
        }
    }
    catch( ... )
    {
    }
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
    // Registering signals handlers
    struct sigaction sa;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;

    // Register the handler for SIGINT.
    sa.sa_handler = signal_handler;
    sigaction( SIGINT, &sa, 0 );
    // Register the handler for SIGTERM
    sa.sa_handler = signal_handler;
    sigaction( SIGTERM, &sa, 0 );
    // Pipe is closed - remote-end hanged
    sa.sa_handler = signal_handler_hungup;
    sigaction( SIGPIPE, &sa, 0 );

    pid_t pid( 0 );
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];
    SOptions options;
    CLogEngine slog;
    CPoDEnvironment env;
    try
    {
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        slog.setDbgFlag( options.m_debug );

        // Show version information
        if( options.m_version )
        {
            version();
            return 0;
        }

        env.init();
        // Start the log engine only on clients (local instances)
        slog.start( env.pipe_log_enginePipeFile() );
    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }

    try
    {
        if( options.m_start || options.m_restart || options.m_stop )
        {
            // send a kill signal to pod-remote daemon if running
            kill_process( env.pod_remotePidFile(), slog );

            if( options.m_stop )
                return 0; // don't need to do anything else
        }

        // Do we need to use the prevues server if there was any...
        if( file_exists( env.pod_remoteCfgFile() ) && options.m_sshConnectionStr.empty() )
        {
            PoD::SPoDRemoteOptions opt_file;
            opt_file.load( env.pod_remoteCfgFile() );
            options.m_sshConnectionStr = opt_file.m_connectionString;
            options.m_sshConnectionStr += ':';
            options.m_sshConnectionStr += opt_file.m_PoDLocation;
            options.m_envScript = opt_file.m_env;
        }

        // the fork stuff we need only if a user wants to send a command
        // to a remote server
        if( !options.m_start && !options.m_stop && !options.m_restart &&
            options.m_command.empty() )
            throw runtime_error( "There is nothing to do.\n" );

        if( options.cleanConnectionString().empty() )
            throw runtime_error( "Please provide a connection URL.\n" );

        // Create pipes, which later will be used for stdout/in/err redirections
        // TODO: close all handles at the end
        if( -1 == pipe( stdin_pipe ) )
            throw system_error( "Error: Can't create a communication stdin_pipe.\n" );
        if( -1 == pipe( stdout_pipe ) )
            throw system_error( "Error: Can't create a communication stdout_pipe\n" );
        if( -1 == pipe( stderr_pipe ) )
            throw system_error( "Error: Can't create a communication stderr_pipe\n" );

        // Create a main ssh tunnel, which serves pod-remote communication.
        // Remote port 22 is so far hard-coded
        CSSHTunnel sshTunnelMain;
        size_t localPortListen( 0 );
        if( !options.m_openDomain.empty() )
        {
            localPortListen = inet::get_free_port( env.getUD().m_server.m_agentPortsRangeMin,
                                                   env.getUD().m_server.m_agentPortsRangeMax );
            sshTunnelMain.deattach();
            sshTunnelMain.setPidFile( env.tunnelRemotePidFile() );
            sshTunnelMain.create( options.cleanConnectionString(), localPortListen,
                                  22, options.m_openDomain );
        }
        // fork a child process
        pid = fork();

        // Make stdout_pipe[0] non-blocking so we can read from it without getting stuck
        fcntl( stdout_pipe[0], F_SETFL, O_NONBLOCK );
        fcntl( stderr_pipe[0], F_SETFL, O_NONBLOCK );

        if( pid == 0 )
        {
            if( !options.m_openDomain.empty() )
            {
                // the current process needs to leave the tunnels open
                sshTunnelMain.deattach();
            }

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
            // tunneled connection string
            string sRemoteConnectionStr;
            // tunneled port
            stringstream ssPort;
            if( !options.m_openDomain.empty() )
            {
                ssPort << "-p" << localPortListen;
                string loginName( options.userNameFromConnectionString() );
                if( !loginName.empty() )
                {
                    sRemoteConnectionStr += loginName;
                    sRemoteConnectionStr += '@';
                }
                sRemoteConnectionStr += "localhost";
            }
            else
            {
                sRemoteConnectionStr = options.cleanConnectionString();
            }
            stringstream ssMsg;
            ssMsg << "child: " << options.cleanConnectionString()
                  << " via a local port " << localPortListen
                  << " on local address " << sRemoteConnectionStr << '\n';
            slog.debug_msg( ssMsg.str() );

            execl( "/usr/bin/ssh", "ssh", "-o StrictHostKeyChecking=no", "-T",
                   ssPort.str().c_str(), sRemoteConnectionStr.c_str(),
                   NULL );

            slog( "Failed to start SSH communication" );
            // we must never come to this point
            exit( 1 );
        }
        else
        {
            stringstream sspid;
            sspid << "forking a child process with pid = " << pid << '\n';
            slog.debug_msg( sspid.str() );
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
                    throw runtime_error( "can't open env. script on the local machine: " + options.m_envScript );

                string env(( istreambuf_iterator<char>( f ) ),
                           istreambuf_iterator<char>() );
                slog.debug_msg( "Executing custom environment script...\n" );
                inet::writeall( stdin_pipe[1], env );
            }

            slog.debug_msg( "Executing PoD environment script...\n" );
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

        if( !options.m_command.empty() )
        {
            // don't stop PoD server if a command failed here
            send_cmd( stdin_pipe[1], options.m_command, false );
            SMessageParserString msg_string;
            CMessageParser<SMessageParserString, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
            msg.parse( msg_string, slog );
            stringstream ss_cmd_resp;
            ss_cmd_resp << "remote end reports: "
                        << msg_string.get() << '\n';
            slog.debug_msg( ss_cmd_resp.str() );
        }
        else if( options.m_start || options.m_restart )
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
                    << "Starting remote PoD server on "
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
                slog.debug_msg( msg_string.get() );
            }
            else
            {
                send_cmd( stdin_pipe[1], "pod-server start" );

                SMessageParserString msg_string;
                CMessageParser<SMessageParserString, CLogEngine> msg( stdout_pipe[0], stderr_pipe[0] );
                msg.parse( msg_string, slog );
                slog.debug_msg( msg_string.get() );
            }
            slog.debug_msg( "Checking service ports...\n" );
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

            stringstream ssPorts;
            ssPorts << "remote PoD ports are as follows: "
                    << agentPort << "(PoD agent), " << xpdPort << "(xpd)" << "\n";
            slog.debug_msg( ssPorts.str() );

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
            // the current process needs to leave the tunnels open
            sshTunnelAgent.deattach();
            sshTunnelAgent.setPidFile( env.tunnelAgentPidFile() );
            sshTunnelAgent.create( options.cleanConnectionString(), agentPortListen,
                                   agentPort, options.m_openDomain );

            CSSHTunnel sshTunnelXpd;
            // the current process needs to leave the tunnels open
            sshTunnelXpd.deattach();
            sshTunnelXpd.setPidFile( env.tunnelXpdPidFile() );
            sshTunnelXpd.create( options.cleanConnectionString(), xpdPortListen,
                                 xpdPort, options.m_openDomain );

            // Store information about the new remote server
            // 1. write server_info.cfg, so that local services can connect to it
            // 2. write remote_server_info.cfg for other pod-remote calls
            PoD::SPoDRemoteOptions opt_file;
            opt_file.m_connectionString = options.cleanConnectionString();
            opt_file.m_PoDLocation = options.remotePoDLocation();
            opt_file.m_env = options.m_envScript;
            opt_file.m_localAgentPort = agentPortListen;
            opt_file.m_localXpdPort = xpdPortListen;
            opt_file.save( env.pod_remoteCfgFile() );

            slog( "Server is started. Use \"pod-info -sd\" to check the status of the server.\n" );
            // daemonize the process

            // Stop the Log Engine
            slog.stop();

            // process ID and Session ID
            pid_t pid_d;
            pid_t sid;

            // Fork off the parent process
            pid_d = ::fork();
            if( pid_d < 0 )
                throw runtime_error( "Can't start pod-remote daemon" );

            // If we got a good PID, then we can exit the parent process.
            if( pid_d > 0 )
                return 0;

            // Change the file mode mask
            ::umask( 0 );

            // Create a new SID for the child process
            sid = ::setsid();
            if( sid < 0 )
                throw runtime_error( "internal error: setsid has failed" );

            // create the pod-remote pid file
            CPIDFile pidfile( env.pod_remotePidFile(), ::getpid() );

            if( !options.m_openDomain.empty() )
            {
                // the current process needs to leave the tunnels open
                sshTunnelMain.attach();
            }

            // attache to the SSH tunnels, so that we can close them when needed
            sshTunnelAgent.attach();
            sshTunnelXpd.attach();

            // start the monitoring
            monitor( stdin_pipe[1], stdout_pipe[0], stderr_pipe[0] );
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
