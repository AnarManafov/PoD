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
//    ( "general.isServerMode", value<bool>( &_Options->m_GeneralData.m_isServerMode )->default_value( true, "yes" ), "todo: desc" )
//    ( "general.work_dir", value<string>( &_Options->m_GeneralData.m_sWorkDir )->default_value( "$POD_LOCATION/" ), "" )
//    ( "general.logfile_dir", value<string>( &_Options->m_GeneralData.m_sLogFileDir )->default_value( "$POD_LOCATION/log" ), "" )
//    ( "general.logfile_overwrite", value<bool>( &_Options->m_GeneralData.m_bLogFileOverwrite )->default_value( false, "no" ), "" )
//    ( "general.log_level", value<size_t>( &_Options->m_GeneralData.m_logLevel )->default_value( 0 ), "" )
//    ( "general.timeout", value<size_t>( &_Options->m_GeneralData.m_nTimeout )->default_value( 0 ), "" )
//    ( "general.proof_cfg_path", value<string>( &_Options->m_GeneralData.m_sPROOFCfg )->default_value( "~/proof.conf" ), "" )
//    ( "general.last_execute_cmd", value<string>( &_Options->m_GeneralData.m_sLastExecCmd ), "" )
//
//    ( "server.listen_port", value<unsigned short>( &_Options->m_serverData.m_nPort )->default_value( 22001 ), "" )
//    ( "server.local_client_port_min", value<unsigned short>( &_Options->m_serverData.m_nLocalClientPortMin )->default_value( 20000 ), "" )
//    ( "server.local_client_port_max", value<unsigned short>( &_Options->m_serverData.m_nLocalClientPortMax )->default_value( 25000 ), "" )
//
//    ( "client.server_port", value<unsigned short>( &_Options->m_clientData.m_nServerPort )->default_value( 22001 ), "" )
//    ( "client.server_addr", value<string>( &_Options->m_clientData.m_strServerHost )->default_value( "lxi020.gsi.de" ), "" )
//    ( "client.local_proofd_port", value<unsigned short>( &_Options->m_clientData.m_nLocalClientPort )->default_value( 111 ), "" )
//    ( "client.shutdown_if_idle_for_sec", value<int>( &_Options->m_clientData.m_shutdownIfIdleForSec )->default_value( 1800 ), "" )
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
