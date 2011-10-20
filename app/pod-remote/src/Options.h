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
    /// this function extracts a PoD Location from the stored connection string
    const std::string remotePoDLocation() const
    {
        std::string::size_type pos = m_sshConnectionStr.find( ":" );
        if( std::string::npos == pos )
            return "";

        return m_sshConnectionStr.substr( pos + 1 );
    }
    /// this function extracts an ssh connection string without the PoD location part
    const std::string cleanConnectionString() const
    {
        std::string::size_type pos = m_sshConnectionStr.find( ":" );
        if( std::string::npos == pos )
            return m_sshConnectionStr;

        return m_sshConnectionStr.substr( 0, pos );
    }
    /// this function extracts an ssh connection string without the PoD location part
    const std::string userNameFromConnectionString() const
    {
        std::string tmp( cleanConnectionString() );
        std::string::size_type pos = tmp.find( "@" );
        if( std::string::npos == pos )
            return std::string();

        return tmp.substr( 0, pos );
    }

    bool m_version;
    bool m_debug;
    bool m_start;
    bool m_stop;
    bool m_restart;
    std::string m_command;
    std::string m_sshConnectionStr;
    std::string m_sshArgs;
    std::string m_openDomain;
    std::string m_envScriptLocal;
    std::string m_envScriptRemote;
};
std::ostream& operator<< ( std::ostream &_stream, const SOptions &_options )
{
    _stream
            << "Current settings:\n"
            << "remote: " << _options.m_sshConnectionStr << "\n"
            << "SSH args: " << _options.m_sshArgs << "\n"
            << "SSH open domain: " << _options.m_openDomain << "\n"
            << "Env. script (local): " << _options.m_envScriptLocal << "\n"
            << "Env. script (remote): " << _options.m_envScriptRemote << "\n";
    return _stream;
}
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
    ( "debug,d", bpo::bool_switch( &( _options->m_debug ) ), "Show debug messages" )
    ( "config,c", bpo::value<std::string>(), "Specify an options file with the pod-remote command line options" )
    ;
    // Connection options
    bpo::options_description connection_options( "Connection options" );
    connection_options.add_options()
    ( "remote", bpo::value<std::string>(), "A connection string including a remote PoD location" )
    ( "ssh-opt", bpo::value<std::string>(), "Additional options, which will be used in SSH commands" )
    ( "ssh-open-domain", bpo::value<std::string>(), "The name of a third party machine open to the outside world"
      " and from which direct connections to the server are possible" )
    ( "env-local", bpo::value<std::string>(), "A full path to environment script (located on the local machine), which will be sourced on the remote end." )
    ( "env-remote", bpo::value<std::string>(), "A full path to environment script (located on the remote machine), which will be sourced on the remote end." )
    ;
    // Commands
    bpo::options_description commands_options( "Commands options" );
    commands_options.add_options()
    ( "start", bpo::bool_switch( &( _options->m_start ) ), "Start remote PoD server" )
    ( "stop", bpo::bool_switch( &( _options->m_stop ) ), "Stop remote PoD server" )
    ( "restart", bpo::bool_switch( &( _options->m_restart ) ), "Restart remote PoD server" )
    ( "command", bpo::value<std::string>(), "Execute arbitrary commands" )
    ;
    // Options for internal use only
    bpo::options_description backend_options( "Backend options" );
//    backend_options.add_options()
//    ( "executor", bpo::bool_switch( &( _options->m_executor ) ), "Switch pod-remote to the executor mode" )
//    ;

    // Declare an options description instance which will include
    // all the options
    bpo::options_description all( "Allowed options" );
    all.add( general_options ).add( connection_options ).add( commands_options ).add( backend_options );

    // Declare an options description instance which will be shown
    // to the user
    bpo::options_description visible( "Allowed options" );
    visible.add( general_options ).add( connection_options ).add( commands_options );

    // Config file options
    bpo::options_description config_file_options( "Allowed options" );
    config_file_options.add( connection_options );

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( all ).run(), vm );
    bpo::notify( vm );

    // Read configuration file
    if( vm.count( "config" ) )
    {
        std::ifstream f( vm["config"].as<std::string>().c_str() );
        if( !f.is_open() )
            throw std::runtime_error( "ERROR: could not read from config file" );

        bpo::store( bpo::parse_config_file( f , config_file_options, true ) , vm );
    }

    boost_hlp::option_dependency( vm, "ssh-opt", "remote" );
    boost_hlp::option_dependency( vm, "ssh-open-domain", "remote" );
    boost_hlp::conflicting_options( vm, "start", "stop" );
    boost_hlp::conflicting_options( vm, "start", "restart" );
    boost_hlp::conflicting_options( vm, "restart", "stop" );
    boost_hlp::conflicting_options( vm, "command", "stop" );
    boost_hlp::conflicting_options( vm, "env-local", "env-remote" );

    if( vm.count( "remote" ) )
    {
        _options->m_sshConnectionStr = vm["remote"].as<std::string>();
        // check that the given url contains a PoD location
        //if( _options->remotePoDLocation().empty() )
        //    throw std::runtime_error( "Bad PoD Location path in the remote url." );

        if( vm.count( "ssh-opt" ) )
        {
            _options->m_sshArgs = vm["ssh-opt"].as<std::string>();
        }
        if( vm.count( "ssh-open-domain" ) )
        {
            _options->m_openDomain = vm["ssh-open-domain"].as<std::string>();
        }

        if( vm.count( "env-local" ) )
        {
            _options->m_envScriptLocal = vm["env-local"].as<std::string>();
            MiscCommon::smart_path( &_options->m_envScriptLocal );
        }
        if( vm.count( "env-remote" ) )
        {
            _options->m_envScriptRemote = vm["env-remote"].as<std::string>();
        }
    }

    if( vm.count( "command" ) )
        _options->m_command = vm["command"].as<std::string>();

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
