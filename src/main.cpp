/************************************************************************/
/**
 * @file main.cpp
 * @brief main file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-06-30
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/cmdline.hpp>
// STD
#include <iostream>
#include <fstream>
#include <string>
// MiscCommon
#include "PoDUserDefaultsOptions.h"
#include "BOOSTHelper.h"

using namespace PoD;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

void printVersion()
{
    // TODO: make VERSION to be taken from the build
    cout << "TODO: print version information" << endl;
}

// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], SPoDUserDefaultsOptions_t *_Options ) throw( exception )
{
    if ( !_Options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "produce help message" )
    ( "version,v", "version information" )
    ( "config,c", bpo::value<string>(), "PoD user-defaults configuration file" )
    ;

    bpo::options_description config_file_options( "PoD user defaults options" );
    config_file_options.add_options()
    ( "general.work_dir", bpo::value<string>( &_Options->m_workDir )->default_value( "$POD_LOCATION/" ), "" )
    ( "general.logfile_dir", bpo::value<string>( &_Options->m_logFileDir )->default_value( "$POD_LOCATION/log" ), "" )
    ( "general.logfile_overwrite", bpo::value<bool>( &_Options->m_logFileOverwrite )->default_value( false, "no" ), "" )
    ( "general.log_level", bpo::value<size_t>( &_Options->m_logLevel )->default_value( 0 ), "" )
    ( "general.agent_timeout", bpo::value<size_t>( &_Options->m_agentTimeout )->default_value( 0 ), "" )
    ( "general.proof_cfg_path", bpo::value<string>( &_Options->m_PROOFCfg )->default_value( "~/proof.conf" ), "" )
    ( "general.last_execute_cmd", bpo::value<string>( &_Options->m_lastExecCmd ), "" )
    ;
    config_file_options.add_options()
    ( "server.agent_server_listen_port", bpo::value<unsigned int>( &_Options->m_agentServerListenPort )->default_value( 22001 ), "" )
    ( "server.agent_server_host", bpo::value<string>( &_Options->m_agentServerHost ), "" )
    ( "server.agent_server_local_client_port_min", bpo::value<unsigned int>( &_Options->m_agentServerLocalClientPortMin )->default_value( 20000 ), "" )
    ( "server.agent_server_local_client_port_max", bpo::value<unsigned int>( &_Options->m_agentServerLocalClientPortMax )->default_value( 25000 ), "" )
    ( "server.xrd_ports_range_min", bpo::value<unsigned int>( &_Options->m_serverXrdPortsRangeMin ) )
    ( "server.xrd_ports_range_max", bpo::value<unsigned int>( &_Options->m_serverXrdPortsRangeMax ) )
    ( "server.xproof_ports_range_min", bpo::value<unsigned int>( &_Options->m_serverXproofPortsRangeMin ) )
    ( "server.xproof_ports_range_max", bpo::value<unsigned int>( &_Options->m_serverXproofPortsRangeMax ) )
    ( "server.agent_server_ports_range_min", bpo::value<unsigned int>( &_Options->m_agentServerPortsRangeMin ) )
    ( "server.agent_server_ports_range_max", bpo::value<unsigned int>( &_Options->m_agentServerPortsRangeMax ) )
    ;
 
   config_file_options.add_options()
    ( "worker.local_proofd_port", bpo::value<unsigned int>( &_Options->m_workerLocalXPROOFPort )->default_value( 111 ), "" )
    ( "worker.shutdown_if_idle_for_sec", bpo::value<int>( &_Options->m_shutdownIfIdleForSec )->default_value( 1800 ), "" )
    ( "worker.xrd_ports_range_min", bpo::value<unsigned int>( &_Options->m_workerXrdPortsRangeMin ) )
    ( "worker.xrd_ports_range_max", bpo::value<unsigned int>( &_Options->m_workerXrdPortsRangeMax ) )
    ( "worker.xproof_ports_range_min", bpo::value<unsigned int>( &_Options->m_workerXproofPortsRangeMin ) )
    ( "worker.xproof_ports_range_max", bpo::value<unsigned int>( &_Options->m_workerXproofPortsRangeMax ) )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    if ( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if ( vm.count( "version" ) )
    {
        printVersion();
        return false;
    }
    if ( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }
    else
    {
        // Load the file and tokenize it
        ifstream ifs( vm["config"].as<string>().c_str() );
        if ( !ifs.good() )
        {
            cout << "Could not open the configuration file" << endl;
            return 1;
        }
        // Parse the config file
        bpo::store( parse_config_file( ifs, config_file_options ), vm );
        bpo::notify( vm );
    }

//    boost_hlp::conflicting_options( vm, "start", "stop" );
//    boost_hlp::conflicting_options( vm, "start", "status" );
//    boost_hlp::conflicting_options( vm, "stop", "status" );
//    boost_hlp::option_dependency( vm, "start", "daemonize" );
//    boost_hlp::option_dependency( vm, "stop", "daemonize" );
//    boost_hlp::option_dependency( vm, "status", "daemonize" );
//
//    if ( vm.count( "start" ) )
//        _Options->m_Command = SOptions_t::Start;
//    if ( vm.count( "stop" ) )
//        _Options->m_Command = SOptions_t::Stop;
//    if ( vm.count( "status" ) )
//        _Options->m_Command = SOptions_t::Status;
//    if ( vm.count( "pidfile" ) )
//    {
//        _Options->m_sPidfileDir = vm["pidfile"].as<string>();
//        // We need to be sure that there is "/" always at the end of the path
//        smart_append( &_Options->m_sPidfileDir, '/' );
//    }
//    _Options->m_bDaemonize = vm.count( "daemonize" );

    return true;
}

int main( int argc, char *argv[] )
{
    // Command line parser
    SPoDUserDefaultsOptions_t Options;
    try
    {
        if ( !parseCmdLine( argc, argv, &Options ) )
            return 0;
    }
    catch ( exception& e )
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return 1;
    }


    return 0;
}
