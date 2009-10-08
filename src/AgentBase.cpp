/************************************************************************/
/**
 * @file AgentBase.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

 version number:     $LastChangedRevision$
 created by:         Anar Manafov
 2009-10-02
 last changed by:    $LastChangedBy$ $LastChangedDate$

 Copyright (c) 2009 GSI GridTeam. All rights reserved.
 *************************************************************************/
// BOOST
#include <boost/bind.hpp>
// STD
#include <csignal>
// MiscCommon
#include "INet.h"
#include "Process.h"
// PROOFAgent
#include "AgentBase.h"

using namespace std;
using namespace MiscCommon;
using namespace PoD;
namespace po = boost::program_options;

sig_atomic_t graceful_quit = 0;

namespace PROOFAgent
{
//=============================================================================
    void signal_handler( int _SignalNumber )
    {
        graceful_quit = 1;
    }

//=============================================================================
    CAgentBase::CAgentBase( const SCommonOptions_t &_common ) :
            m_commonOptions( _common ), m_agentServerListenPort( 0 )
    {
        // Registering signals handlers
        struct sigaction sa;
        sigemptyset( &sa.sa_mask );
        sa.sa_flags = 0;

        // Register the handler for SIGINT.
        sa.sa_handler = signal_handler;
        sigaction( SIGINT, &sa, 0 );
        // Register the handler for SIGTERM
        sa.sa_handler = signal_handler;
        sigaction( SIGTERM, &sa, 0 );
    }

//=============================================================================
    CAgentBase::~CAgentBase()
    {
    	delete m_monitorThread;
        // deleting proof configuration file
        if ( !m_commonOptions.m_proofCFG.empty() )
            ::unlink( m_commonOptions.m_proofCFG.c_str() );
    }

//=============================================================================
    void CAgentBase::readServerInfoFile( const string &_filename )
    {
        po::variables_map keys;
        po::options_description options(
            "Agent's server info config" );
        // HACK: Don't make a long add_options, otherwise Eclipse 3.5's CDT indexer can't handle it
        options.add_options()
        ( "server.host", po::value<string>(), "" )
        ( "server.port", po::value<unsigned int>(), "" );
        ifstream ifs( _filename.c_str() );
        if ( !ifs.is_open() || !ifs.good() )
        {
            string msg( "Could not open a server info configuration file: " );
            msg += _filename;
            throw runtime_error( msg );
        }
        // Parse the config file
        po::store( po::parse_config_file( ifs, options ), keys );
        po::notify( keys );
        if ( keys.count( "server.host" ) )
            m_agentServerHost = keys["server.host"].as<string> ();
        if ( keys.count( "server.port" ) )
            m_agentServerListenPort = keys["server.port"].as<unsigned int> ();
    }

//=============================================================================
    MiscCommon::ERRORCODE CAgentBase::Start()
    {
    	// start a monitoring job
    	 m_monitorThread = new boost::thread( boost::bind( &CAgentBase::monitor, this ) );

     	// start the main job
     	run();

        return erOK;
    }

//=============================================================================
    bool CAgentBase::IsPROOFReady( unsigned short _Port ) const
    {
        // #1  - Check whether xrootd process exists
        vectorPid_t pids = getprocbyname( "xrootd" );
        if ( pids.empty() )
            return false;

        vectorPid_t::const_iterator iter = pids.begin();
        vectorPid_t::const_iterator iter_end = pids.end();
        // checking that the process is running under current's user id
        bool found = false;
        for ( ; iter != iter_end; ++iter )
        {
            CProcStatus p;
            p.Open( *iter );
            istringstream ss( p.GetValue( "Uid" ) );
            uid_t realUid( 0 );
            ss >> realUid;
            if ( getuid() == realUid )
            {
                found = true;
                break;
            }
        }

        if ( !found )
            return false;
        // #2 - Check whether PROOF port is in use
        if ( 0 == _Port )
            return true;
        if ( 0 != MiscCommon::INet::get_free_port( _Port ) )
            return false; // PROOF port is not in use, that means we can't connect to it
        // TODO: Implement more checks of xrootd/proof here
        return true;
    }

}
