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
// pod-ssh
#include "version.h"
#include "config.h"
#include "worker.h"
#include "logEngine.h"

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
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

    if( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }

    boost_hlp::conflicting_options( vm, "submit", "clean" );

    _vm->swap( vm );
    return true;
}
//=============================================================================
int main( int argc, char * argv[] )
{
    CLogEngine slog;
    log_func_t log_fun_ptr = boost::bind( &CLogEngine::operator(), &slog, _1, _2 );
    bpo::variables_map vm;
    try
    {
        if( !parseCmdLine( argc, argv, &vm ) )
            return 0;
        ifstream f( vm["config"].as<string>().c_str() );
        if( !f.is_open() )
        {
            string msg( "can't open configuration file \"" );
            msg += vm["config"].as<string>();
            msg += "\"";
            throw runtime_error( msg );
        }

        // Collect workers list
        slog.start();
        typedef list<CWorker> workersList_t;
        size_t wrkCount( 0 );
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

                wrkCount += rec->m_nWorkers;
            }
        }

        // some controle information
        ostringstream ss;
        ss << "Number of PoD workers: " << workers.size() << "\n";
        slog( ss.str() );
        ss.str( "" );
        ss << "Number of PROOF workers: " << wrkCount << "\n";
        slog( ss.str() );

        // it's a dry run - configuration check only
        if( !vm.count( "submit" ) && !vm.count( "clean" ) )
            throw runtime_error( "It's a configuration check only. Specify submit/clean options to actually execute." );

        slog( "Workers list:\n" );

        // start threadpool and push tasks into it
        CThreadPool<CWorker, ETaskType> threadPool( 4 );
        ETaskType task_type = ( vm.count( "submit" ) ) ? task_submit : task_clean;
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
    }
    catch( exception& e )
    {
        slog( e.what() );
        return 1;
    }
    return 0;
}
