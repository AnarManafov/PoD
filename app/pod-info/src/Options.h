//
//  Options.h
//  PoD
//
//  Created by Anar Manafov on 02.02.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef OPTIONS_H
#define OPTIONS_H
//=============================================================================
// STD
#include <stdexcept>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// MiscCommon
#include "BOOSTHelper.h"
//=============================================================================
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
struct SOptions
{
    SOptions():
        m_version( false ),
        m_connectionString( false ),
        m_listWNs( false ),
        m_countWNs( false ),
        m_status( false ),
        m_debug( false ),
        m_batchMode( false ),
        m_xpdPid( false ),
        m_xpdPort( false ),
        m_agentPid( false ),
        m_agentPort( false )
    {
    }

    bool m_version;
    bool m_connectionString;
    bool m_listWNs;
    bool m_countWNs;
    bool m_status;
    bool m_debug;
    std::string m_sshConnectionStr;
    std::string m_sshArgs;
    std::string m_openDomain;
    bool m_batchMode;
    std::string m_remotePath;
    bool m_xpdPid;
    bool m_xpdPort;
    bool m_agentPid;
    bool m_agentPort;
};
//=============================================================================
// Command line parser
inline bool parseCmdLine( int _Argc, char *_Argv[], SOptions *_options ) throw( std::exception )
{
    if( !_options )
        throw std::runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description general_options( "General options" );
    general_options.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", bpo::bool_switch( &( _options->m_version ) ), "Version information" )
    ( "batch,b", bpo::bool_switch( &( _options->m_batchMode ) ), "Enable the batch mode" )
    ( "debug,d", bpo::bool_switch( &( _options->m_debug ) ), "Show debug messages" )
    ;
    // Connection options
    bpo::options_description connection_options( "Connection options" );
    connection_options.add_options()
    ( "remote", bpo::value<std::string>(), "An SSH connection string. Directs pod-info to use SSH to detect a remote PoD server" )
    ( "remote_path", bpo::value<std::string>(), "A working directory of the remote PoD server"
      " It is very important either to write an explicit path or use quotes, so that shell will not substitute local variable in the remote path. (default: ~/.PoD/)" )
    ( "ssh_opt", bpo::value<std::string>(), "Additional options, which will be used in SSH commands" )
    ( "ssh_open_domain", bpo::value<std::string>(), "The name of a third party machine open to the outside world"
      " and from which direct connections to the server are possible" )
    ;
    // Information options
    bpo::options_description information_options( "Information options" );
    information_options.add_options()
    ( "connection_string,c", bpo::bool_switch( &( _options->m_connectionString ) ), "Show PROOF connection string." )
    ( "list,l", bpo::bool_switch( &( _options->m_listWNs ) ), "List all available PROOF workers." )
    ( "number,n", bpo::bool_switch( &( _options->m_countWNs ) ), "Report a number of currently available PROOF workers." )
    ( "status,s", bpo::bool_switch( &( _options->m_status ) ), "Show status of PoD server." )
    ( "xpdPid", bpo::bool_switch( &( _options->m_xpdPid ) ), "Show the process ID of the local xproofd" )
    ( "xpdPort", bpo::bool_switch( &( _options->m_xpdPort ) ), "Show the port number of the local xproofd" )
    ( "agentPid", bpo::bool_switch( &( _options->m_agentPid ) ), "Show the process ID of the local pod-agent server" )
    ( "agentPort", bpo::bool_switch( &( _options->m_agentPort ) ), "Show the port number of the local pod-agent server" )
    ;
    // Declare an options description instance which will be shown
    // to the user
    bpo::options_description visible( "Allowed options" );
    visible.add( general_options ).add( connection_options ).add( information_options );

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    boost_hlp::option_dependency( vm, "ssh_opt", "remote" );

    if( vm.count( "remote" ) )
    {
        _options->m_sshConnectionStr = vm["remote"].as<std::string>();
        _options->m_remotePath = ( vm.count( "remote_path" ) ) ?
                                 vm["remote_path"].as<std::string>() : "~/.PoD/";
        MiscCommon::smart_append( &_options->m_remotePath, '/' );

        if( vm.count( "ssh_opt" ) )
        {
            _options->m_sshArgs = vm["ssh_opt"].as<std::string>();
        }
        if( vm.count( "ssh_open_domain" ) )
        {
            _options->m_openDomain = vm["ssh_open_domain"].as<std::string>();
        }
    }

    // if there are no arguments is given, produce a help message
    bpo::variables_map::const_iterator iter = vm.begin();
    bpo::variables_map::const_iterator iter_end = vm.end();
    bool show_help( true );
    for( ; iter != iter_end; ++iter )
    {
        if( !iter->second.defaulted() )
            show_help = false;
    }
    if( vm.count( "help" ) || show_help )
    {
        std::cout << visible << std::endl;
        return false;
    }

    return true;
}

#endif
