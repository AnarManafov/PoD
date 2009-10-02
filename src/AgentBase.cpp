/************************************************************************/
/**
 * @file AgentBase.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "AgentBase.h"

using namespace std;

sig_atomic_t graceful_quit = 0;

namespace PROOFAgent
{
//=============================================================================
    void signal_handler( int _SignalNumber )
    {
        graceful_quit = 1;
    }
//=============================================================================
//------------------------- Agent Base class --------------------------------------------------------
    void CAgentBase::readServerInfoFile( const string &_filename )
    {
        boost::program_options::variables_map keys;
        boost::program_options::options_description options( "Agent's server info config" );
        // HACK: Don't make a long add_options, otherwise Eclipse 3.5's CDT indexer can't handle it
        options.add_options()
        ( "server.host", boost::program_options::value<std::string>(), "" )
        ( "server.port", boost::program_options::value<unsigned int>(), "" )
        ;
        std::ifstream ifs( _filename.c_str() );
        if ( !ifs.is_open() || !ifs.good() )
        {
            string msg( "Could not open a server info configuration file: " );
            msg += _filename;
            throw runtime_error( msg );
        }
        // Parse the config file
        boost::program_options::store( boost::program_options::parse_config_file( ifs, options ), keys );
        boost::program_options::notify( keys );
        if ( keys.count( "server.host" ) )
            m_agentServerHost = keys["server.host"].as<string>();
        if ( keys.count( "server.port" ) )
            m_agentServerListenPort = keys["server.port"].as<unsigned int>();
    }

}
