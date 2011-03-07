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
        m_debug( false ),
        m_start( false ),
        m_stop( false ),
        m_restart( false )
    {
    }
    bool operator== ( const SOptions &_val )
    {
        return ( m_version == _val.m_version &&
                 m_debug == _val.m_debug &&
                 m_start == _val.m_start &&
                 m_stop == _val.m_stop &&
                 m_restart == _val.m_restart &&
                 m_sshConnectionStr == _val.m_sshConnectionStr &&
                 m_sshArgs == _val.m_sshArgs &&
                 m_openDomain == _val.m_openDomain &&
                 m_remotePath == _val.m_remotePath );
    }

    bool m_version;
    bool m_debug;
    bool m_start;
    bool m_stop;
    bool m_restart;
    std::string m_sshConnectionStr;
    std::string m_sshArgs;
    std::string m_openDomain;
    std::string m_remotePath;
};
//=============================================================================
// Command line parser
inline bool parseCmdLine( int _Argc, char *_Argv[], SOptions *_options ) throw( std::exception )
{
    if( !_options )
        throw std::runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", bpo::bool_switch( &( _options->m_version ) ), "Version information." )
    ( "debug,d", bpo::bool_switch( &( _options->m_debug ) ), "Show debug messages." )
    ( "remote", bpo::value<std::string>(), "An SSH connection string. Directs pod-info to use SSH to detect a remote PoD server." )
    ( "remote_path", bpo::value<std::string>(), "A working directory of the remote PoD server."
      " It is very important either to write an explicit path or use quotes, so that shell will not substitute local variable in the remote path. (default: ~/.PoD/)" )
    ( "ssh_opt", bpo::value<std::string>(), "Additional options, which will be used in SSH commands." )
    ( "ssh_open_domain", bpo::value<std::string>(), "The name of a third party machine open to the outside world"
      " and from which direct connections to the server are possible." )
    ( "start", bpo::bool_switch( &( _options->m_start ) ), "Start remote PoD server." )
    ( "stop", bpo::bool_switch( &( _options->m_stop ) ), "Stop remote PoD server." )
    ( "restart", bpo::bool_switch( &( _options->m_start ) ), "Restart remote PoD server." )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    boost_hlp::option_dependency( vm, "start", "remote" );
    boost_hlp::option_dependency( vm, "stop", "remote" );
    boost_hlp::option_dependency( vm, "restart", "remote" );
    boost_hlp::option_dependency( vm, "ssh_opt", "remote" );
    boost_hlp::option_dependency( vm, "remote_path", "remote" );
    boost_hlp::option_dependency( vm, "ssh_open_domain", "remote" );

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

    // we need an empty struct to check the case when user don't provide any argument
    SOptions s;
    if( vm.count( "help" ) || ( s == *_options ) )
    {
        std::cout << visible << std::endl;
        return false;
    }

    return true;
}

#endif
