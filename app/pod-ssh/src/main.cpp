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

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
const LPCSTR g_pipeName = ".pod_ssh_pipe";
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
    // TODO: we need to be able to clean only selected worker(s)
    // At this moment we clean all workers.
    ( "clean", "Clean all workers" )
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
    boost_hlp::conflicting_options( vm, "status", "submit" );

    _vm->swap( vm );
    return true;
}
//=============================================================================
void repackPkg( string *_cmdOutput )
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
        smart_path( &cmd );
        string arg( "source " );
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;
//       execl( "/bin/bash", "bash", "-c", arg.c_str(), NULL );

        StringVector_t params;
        //     params.push_back( "bash" );
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
    CLogEngine slog;
    CEnvironment env;
    env.init();
    // convert log engine's functor to a free call-back function
    // this is needed to pass the logger further to other objects
    log_func_t log_fun_ptr = boost::bind( &CLogEngine::operator(), &slog, _1, _2, _3 );
    bpo::variables_map vm;
    try
    {
        // a number of threads in the thread-pool
        // default is 4 and the maximum is 50.
        // These are just magic numbers. We want to revise them later on...
        size_t nThreads( env.getUD().m_server.m_agentThreads );
        if( nThreads <= 4 || nThreads > 50 )
            nThreads = 4;

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


        string configFile;
        if( !vm.count( "config" ) )
        {
            PoD::SPoDSSHOptions opt_file;
            opt_file.load( env.pod_sshCfgFile() );
            configFile = opt_file.m_config;
        }
        else
        {
            configFile = vm["config"].as<string>();
            smart_path( &configFile );
        }

        ifstream f( configFile.c_str() );
        if( !f.is_open() )
        {
            string msg( "can't open configuration file \"" );
            msg += vm["config"].as<string>();
            msg += "\"";
            throw runtime_error( msg );
        }

        // Collect workers list
        string pipeName( env.getUD().m_server.m_common.m_workDir );
        smart_append( &pipeName, '/' );
        pipeName += g_pipeName;
        slog.start( pipeName );

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
                do_execv( cmd, params, 2, &output );
            }
            catch( exception &e )
            {
                throw runtime_error( "PoD server is NOT running. Please, start PoD server first." );
            }

            // re-pack PoD worker package if needed
            string cmdOutput;
            repackPkg( &cmdOutput );
            stringstream ss( cmdOutput );
            // send the output line by line to the log
            StringVector_t vec;
            std::copy( custom_istream_iterator<std::string>( ss ),
                       custom_istream_iterator<std::string>(),
                       std::back_inserter( vec ) );
            StringVector_t::const_iterator iter = vec.begin();
            StringVector_t::const_iterator iter_end = vec.end();
            for( ; iter != iter_end; ++iter )
                slog( *iter + '\n' );
        }

        typedef list<CWorker> workersList_t;
        size_t wrkCount( 0 );
        bool dynWrk( false );
        workersList_t workers;
        {
            CConfig config;
            config.readFrom( f );

            configRecords_t recs( config.getRecords() );
            configRecords_t::const_iterator iter = recs.begin();
            configRecords_t::const_iterator iter_end = recs.end();
            for( ; iter != iter_end; ++iter )
            {
                configRecord_t rec( *iter );
                CWorker wrk( rec, log_fun_ptr );
                workers.push_back( wrk );

                if( 0 == rec->m_nWorkers )
                    dynWrk = true; // user wants us to dynamicly decide on how many PROOF workers to create

                wrkCount += rec->m_nWorkers;
            }
        }

        // some controle information
        ostringstream ss;
        ss << "There are " << nThreads << " threads in the tread-pool.\n";
        slog( ss.str() );
        ss.str( "" );
        ss << "Number of PoD workers: " << workers.size() << "\n";
        slog( ss.str() );
        ss.str( "" );
        if( dynWrk )
            ss << "Number of PROOF workers: on some workers is dynamic, according to a number of CPU cores\n";
        else
            ss << "Number of PROOF workers: " << wrkCount << "\n";
        slog( ss.str() );

        // it's a dry run - configuration check only
        if( !vm.count( "submit" ) && !vm.count( "clean" ) && !vm.count( "status" ) )
            throw runtime_error( "It's a configuration check only. Specify submit/clean options to actually execute." );

        slog( "Workers list:\n" );

        // start thread-pool and push tasks into it
        CThreadPool<CWorker, ETaskType> threadPool( nThreads );
        ETaskType task_type( task_submit );
        if( vm.count( "clean" ) )
            task_type = task_clean;
        else if( vm.count( "status" ) )
            task_type = task_status;

        workersList_t::iterator iter = workers.begin();
        workersList_t::iterator iter_end = workers.end();
        for( ; iter != iter_end; ++iter )
        {
            ostringstream ss;
            iter->printInfo( ss );
            ss << "\n";
            slog( ss.str().c_str() );
            threadPool.pushTask( *iter, task_type );
        }
        threadPool.stop( true );

        // Check the status of all tasks
        iter = workers.begin();
        iter_end = workers.end();
        size_t g( 0 );
        size_t b( 0 );
        for( ; iter != iter_end; ++iter )
        {
            if( iter->IsLastTaskSuccess() )
                ++g;
            else
                ++b;
        }
        ostringstream msg;
        msg
                << "\n*******************\n"
                << "Successfully processed tasks: " << g << '\n'
                << "Failed tasks: " << b << '\n'
                << "*******************\n";
        slog( msg.str() );

        PoD::SPoDSSHOptions opt_file;
        opt_file.m_config = configFile;
        opt_file.save( env.pod_sshCfgFile() );
    }
    catch( exception& e )
    {
        slog( e.what() + string( "\n" ) );
        return 1;
    }


    return 0;
}
