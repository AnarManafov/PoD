/************************************************************************/
/**
 * @file main.cpp
 * @brief Implementation of the "Main" function
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-05-17
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI, Scientific Computing group. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// STD
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>
// MiscCommon
#include "BOOSTHelper.h"
#include "SysHelper.h"
#include "PoDUserDefaultsOptions.h"
#include "PoDSysFiles.h"
// pod-ssh
#include "version.h"
#include "config.h"
#include "worker.h"
#include "logEngine.h"
#include "Process.h"
#include "local_types.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
const LPCSTR g_pipeName = ".pod_ssh_pipe";
typedef list<CWorker> workersList_t;
typedef CThreadPool<CWorker, ETaskType> threadPool_t;
//=============================================================================
void printVersion()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], bpo::variables_map *_vm )
{
    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "config,c", bpo::value<string>(), "PoD's ssh plug-in configuration file" )
    ( "submit", "Submit workers" )
    ( "status", "Request status of the workers" )
    ( "exec,e", bpo::value<string>(), "Execute a local shell script on the remote worker nodes" )
    // TODO: we need to be able to clean only selected worker(s)
    // At this moment we clean all workers.
    ( "clean", "Clean all workers" )
    ( "fast-clean", "The fast version of the clean procedure."
      " It shuts worker nodes down. It doesn't actually clean workers' directories." )
    ( "logs", "Download all log files from the worker nodes. Can be used only together with the clean option" )
    ( "debug,d", "Verbose mode. Causes pod-ssh to print debugging messages about its progress" )
    ( "threads,t", bpo::value<size_t>()->default_value( 5 ),
      "It defines a number of threads in pod-ssh's thread pool."
      " Min value is 1, max value is (Core*2)."
      " Default: 5" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    if( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if( vm.count( "version" ) )
    {
        printVersion();
        return false;
    }


    boost_hlp::conflicting_options( vm, "submit", "clean" );
    boost_hlp::conflicting_options( vm, "status", "clean" );
    boost_hlp::conflicting_options( vm, "submit", "fast-clean" );
    boost_hlp::conflicting_options( vm, "status", "fast-clean" );
    boost_hlp::conflicting_options( vm, "clean", "fast-clean" );
    boost_hlp::conflicting_options( vm, "status", "submit" );
    boost_hlp::option_dependency( vm, "logs", "clean" );

    _vm->swap( vm );
    return true;
}
//=============================================================================
void repackPkg( string *_cmdOutput, bool _needInlineBashScript = false )
{
    // re-create the worker package if needed
    string out;
    try
    {
        // invoking a new bash process can in some case overwrite env. vars
        // To be shure that our env is there, we call PoD_env.sh
        string cmd_env( "$POD_LOCATION/PoD_env.sh" );
        smart_path( &cmd_env );
        string cmd( "$POD_LOCATION/bin/pod-prep-worker" );
        if(_needInlineBashScript)
            cmd += " -i";
        smart_path( &cmd );
        string arg( "source " );
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;

        StringVector_t params;
        params.push_back( "-c" );
        params.push_back( arg );
        // 10 sec time-out for this command
        do_execv( "/bin/bash", params, 10, &out );
    }
    catch( exception &e )
    {
        string msg( "Can't create PoD worker package: " );
        msg += out;
        throw runtime_error( msg );
    }
    if( _cmdOutput )
        *_cmdOutput = out;
}
//=============================================================================
int main( int argc, char * argv[] )
{
    bpo::variables_map vm;
    try
    {
        if( !parseCmdLine( argc, argv, &vm ) )
            return 0;
    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }

    CLogEngine slog( vm.count( "debug" ) );
    try
    {
        // convert log engine's functor to a free call-back function
        // this is needed to pass the logger further to other objects
        log_func_t log_fun_ptr = boost::bind( &CLogEngine::operator(),
                                              &slog, _1, _2, _3 );

        CPoDEnvironment env;
        env.init();

        // Collect workers list
        string pipeName( env.getUD().m_server.m_common.m_workDir );
        smart_append( &pipeName, '/' );
        pipeName += g_pipeName;
        slog.start( pipeName );

        string configFile;
        if( !vm.count( "config" ) )
        {
#if defined (BOOST_PROPERTY_TREE)
            PoD::SPoDSSHOptions opt_file;
            opt_file.load( env.pod_sshCfgFile() );
            configFile = opt_file.m_config;
#else
            throw runtime_error( "Error: missing argument: PoD SSH config file is not specified." );
#endif
        }
        else
        {
            configFile = vm["config"].as<string>();
            smart_path( &configFile );
        }

        if( !file_exists( configFile ) )
            throw runtime_error( "PoD SSH config file doesn't exist: " + configFile );

        ifstream f( configFile.c_str() );
        if( !f.is_open() )
        {
            string msg( "can't open configuration file \"" );
            msg += vm["config"].as<string>();
            msg += "\"";
            throw runtime_error( msg );
        }

        // Check that PoD server is running
        if( vm.count( "submit" ) )
        {
            try
            {
                string cmd( "$POD_LOCATION/bin/pod-server" );
                smart_path( &cmd );
                StringVector_t params;
                params.push_back( "status_with_code" );
                string output;
                string errout;
                do_execv( cmd, params, 2, &output, &errout );
            }
            catch( exception &e )
            {
                throw runtime_error( "PoD server is NOT running. Please, start PoD server first." );
            }

            // re-pack PoD worker package if needed
            // We will call repack once again if there is an inline bash script
            string cmdOutput;
            repackPkg( &cmdOutput);
            stringstream ss( cmdOutput );
            // send the output line by line to the log
            StringVector_t vec;
            std::copy( custom_istream_iterator<std::string>( ss ),
                       custom_istream_iterator<std::string>(),
                       std::back_inserter( vec ) );
            StringVector_t::const_iterator iter = vec.begin();
            StringVector_t::const_iterator iter_end = vec.end();
            for( ; iter != iter_end; ++iter )
                slog.debug_msg( *iter + '\n' );
        }

        size_t wrkCount( 0 );
        bool dynWrk( false );
        SWNOptions options;
        options.m_debug = vm.count( "debug" );
        options.m_logs = vm.count( "logs" );
        options.m_fastClean = vm.count( "fast-clean" );
        if( vm.count( "exec" ) )
            options.m_scriptName = vm["exec"].as<string>();

        workersList_t workers;
        string inlineShellScripCmds;
        {
            CConfig config;
            config.readFrom( f );
            inlineShellScripCmds = config.getBashEnvCmds();

            configRecords_t recs( config.getRecords() );
            configRecords_t::const_iterator iter = recs.begin();
            configRecords_t::const_iterator iter_end = recs.end();
            for( ; iter != iter_end; ++iter )
            {
                configRecord_t rec( *iter );
                CWorker wrk( rec, &log_fun_ptr, options );
                workers.push_back( wrk );

                if( 0 == rec->m_nWorkers )
                    dynWrk = true; // user wants us to dynamicly decide on how many PROOF workers to create

                wrkCount += rec->m_nWorkers;
            }
        }

        // Need to repack worker package
        // in order to insert a user defined shell script
        if( !inlineShellScripCmds.empty() )
        {
            slog.debug_msg( "An inline shell script is found. Inserting it into wrk. package\n" );
            string scriptFileName( PoD::showWrkPackageDir() );
            scriptFileName += "user_worker_env.sh";
            smart_path( &scriptFileName );

            ofstream f_script( scriptFileName.c_str() );
            if( !f_script.is_open() )
                throw runtime_error( "Can't open for writing: " + scriptFileName );

            f_script << inlineShellScripCmds;
            f_script.close();
            
            // re-pack PoD worker package if needed
            string cmdOutput;
            repackPkg( &cmdOutput, !inlineShellScripCmds.empty() );
            stringstream ss( cmdOutput );
            // send the output line by line to the log
            StringVector_t vec;
            std::copy( custom_istream_iterator<std::string>( ss ),
                      custom_istream_iterator<std::string>(),
                      std::back_inserter( vec ) );
            StringVector_t::const_iterator iter = vec.begin();
            StringVector_t::const_iterator iter_end = vec.end();
            for( ; iter != iter_end; ++iter )
                slog.debug_msg( *iter + '\n' );

        }
        else if( vm.count( "submit" ) )
        {
            stringstream ssWarning;
            ssWarning
                    << "\n********************************************\n"
                    << "Warning! There is no inline environment script found in "
                    << configFile << "\n"
                    << "Be advised, with SSH plug-in it is very often the case,\n"
                    << "that PoD can't start workers, because xproofd/ROOT is not\n"
                    << "in the PATH on worker nodes.\n"
                    << "If your PoD job fails, just after submission it shows DONE status, \n"
                    << "then you may want use inline environment script in your pod-ssh config file.\n"
                    << "See http://pod.gsi.de/doc/pro/pod-ssh.html for more information\n"
                    << "\n"
                    << "Usage of user_worker_env.sh in pod-ssh is deprecated.\n"
                    << "********************************************\n";
            slog( ssWarning.str() );
        }

        // a number of threads in the thread-pool
        //size_t nThreads( env.getUD().m_server.m_agentThreads );
        size_t nThreads( vm["threads"].as<size_t>() );
        // Protection for a number of threads
        if( nThreads <= 0 || nThreads > ( getNCores() * 2 ) )
        {
            slog.debug_msg( "Warning: bad number of threads. The default will be used.\n" );
            nThreads = 5;
        }
        // some control information
        ostringstream ss;
        ss << "There are " << nThreads << " threads in the tread-pool.\n";
        slog.debug_msg( ss.str() );
        ss.str( "" );
        ss << "Number of PoD workers: " << workers.size() << "\n";
        slog.debug_msg( ss.str() );
        ss.str( "" );
        if( dynWrk )
            ss << "Number of PROOF workers: on some workers is dynamic, according to a number of CPU cores\n";
        else
            ss << "Number of PROOF workers: " << wrkCount << "\n";
        slog.debug_msg( ss.str() );

        // it's a dry run - configuration check only
        if( !vm.count( "submit" ) && !vm.count( "clean" ) &&
            !vm.count( "status" ) && !vm.count( "fast-clean" ) &&
            !vm.count( "exec" ) )
            throw runtime_error( "Running in dry mode. It's a configuration check only mode." );

        slog.debug_msg( "Workers list:\n" );

        // start thread-pool and push tasks into it
        threadPool_t threadPool( nThreads );

        workersList_t::iterator iter = workers.begin();
        workersList_t::iterator iter_end = workers.end();
        for( ; iter != iter_end; ++iter )
        {
            ostringstream ss;
            iter->printInfo( ss );
            ss << "\n";
            slog.debug_msg( ss.str().c_str() );

            // pash pre-tasks
            if( vm.count( "exec" ) )
                threadPool.pushTask( *iter, task_exec );

            // push main tasks
            if( vm.count( "clean" ) || vm.count( "fast-clean" ) )
                threadPool.pushTask( *iter, task_clean );
            else if( vm.count( "status" ) )
                threadPool.pushTask( *iter, task_status );
            else if( vm.count( "submit" ) )
                threadPool.pushTask( *iter, task_submit );
        }
        threadPool.stop( true );

        // Check the status of all tasks Failed
        size_t badFailedCount = threadPool.tasksCount() - threadPool.successfulTasks();
        ostringstream msg;
        msg
                << "\n*******************\n"
                << "Successfully processed tasks: " << threadPool.successfulTasks() << '\n'
                << "Failed tasks: " << badFailedCount << '\n'
                << "*******************\n";
        slog.debug_msg( msg.str() );

        if( badFailedCount > 0 && !vm.count( "debug" ) )
            slog( "WARNING: some tasks have failed. Please use the \"--debug\""
                  " option to print debugging messages.\n" );

        if( !vm.count( "debug" ) )
            slog( "PoD jobs have been submitted. Use \"pod-ssh --status\" to check the status.\n" );

#if defined (BOOST_PROPERTY_TREE)
        PoD::SPoDSSHOptions opt_file;
        opt_file.m_config = configFile;
        opt_file.save( env.pod_sshCfgFile() );
#endif
    }
    catch( exception& e )
    {
        slog( e.what() + string( "\n" ) );
        return 1;
    }


    return 0;
}
