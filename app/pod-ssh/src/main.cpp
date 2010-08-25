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

using namespace std;
using namespace MiscCommon;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
int fdSignalPipe( 0 );
string signalPipeName;
void log_engine();
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
void closePipe()
{
    close( fdSignalPipe );
    fdSignalPipe = 0;
    unlink( signalPipeName.c_str() );
}
//=============================================================================
int main( int argc, char *argv[] )
{
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

        // create a named pipe (our signal pipe)
        // it's use to collect outputs from the threads and called shell scripts...
        signalPipeName = "$POD_LOCATION/";
        smart_path( &signalPipeName );
        signalPipeName += ".ssh_plugin_pipe";
        int ret_val = mkfifo( signalPipeName.c_str(), 0666 );
        if(( -1 == ret_val ) && ( EEXIST != errno ) )
        {
            ostringstream ss;
            ss
                    << "Can't create a named pipe: "
                    << signalPipeName;
            cerr << PROJECT_NAME << ": " << ss.str() << endl;
        }

        // Open the pipe for reading
        fdSignalPipe = open( signalPipeName.c_str(), O_RDWR | O_NONBLOCK );
        if(( -1 == fdSignalPipe ) && ( EEXIST != errno ) )
        {
            ostringstream ss;
            ss
                    << "Can't open a named pipe: "
                    << signalPipeName;
            cerr << PROJECT_NAME << ": " << ss.str() << endl;
        }

        // Start the log engine
        boost::thread monitorThread( log_engine );

        // Collect workers list
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
                CWorker wrk( rec, fdSignalPipe );
                workers.push_back( wrk );

                wrkCount += rec->m_nWorkers;
            }
        }

        // some controle information
        {
            ostringstream ss;
            ss << PROJECT_NAME << ": " << "Number of PoD workers: " << workers.size() << "\n"
               << PROJECT_NAME << ": " << "Number of PROOF workers: " << wrkCount << "\n"
               << PROJECT_NAME << ": " << "Workers list:\n";
            write( fdSignalPipe, ss.str().c_str(), ss.str().size() );
        }

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
            write( fdSignalPipe, ss.str().c_str(), ss.str().size() );
            threadPool.pushTask( *iter, task_type );
        }
        threadPool.stop( true );
    }
    catch( exception& e )
    {
        closePipe();
        cerr << PROJECT_NAME << ": error: " << e.what() << endl;
        return 1;
    }

    closePipe();
    return 0;
}
//=============================================================================
void log_engine()
{
    while( fdSignalPipe > 0 )
    {
        fd_set readset;
        FD_ZERO( &readset );
        FD_SET( fdSignalPipe, &readset );
        int retval = ::select( fdSignalPipe + 1, &readset, NULL, NULL, NULL );

        if( EBADF == errno )
            return;

        if( retval < 0 )
        {
            cerr << PROJECT_NAME << ": Problem in the log engine: " << errno2str() << endl;
            return;
        }

        if( FD_ISSET( fdSignalPipe, &readset ) )
        {
            const int read_size = 64;
            char buf[read_size];
            int numread( 0 );
            while( true )
            {
                numread = read( fdSignalPipe, buf, read_size );
                if( numread > 0 )
                    cout << string( buf, numread );
                else
                    break;
            }
            cout.flush();
        }
    }
}
