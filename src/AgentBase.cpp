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

        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "AgentBase.h"
// API
#include <sys/types.h>
#include <sys/stat.h>
// BOOST
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
// STD
#include <csignal>
// MiscCommon
#include "INet.h"
#include "Process.h"
#include "SysHelper.h"

//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace PoD;
namespace po = boost::program_options;
//=============================================================================
sig_atomic_t graceful_quit = 0;
//=============================================================================
namespace PROOFAgent
{
//=============================================================================
    void signal_handler( int _SignalNumber )
    {
        graceful_quit = 1;
    }

//=============================================================================
    CAgentBase::CAgentBase( const SCommonOptions_t &_common ) :
            m_commonOptions( _common ),
            m_agentServerListenPort( 0 ),
            m_fdSignalPipe( 0 )
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

        // create a named pipe (our signal pipe)
        // it's use to interrupt "select" and give a chance to new sockets to be added
        // to the "select"
        // using a fixed name ".signal_pipe" and server.work_dir as a path
        m_signalPipeName = _common.m_workDir;
        smart_path( &m_signalPipeName );
        smart_append( &m_signalPipeName, '/' );
        m_signalPipeName += ".signal_pipe";
        int ret_val = mkfifo( m_signalPipeName.c_str(), 0666 );
        if (( -1 == ret_val ) && ( EEXIST != errno ) )
        {
            ostringstream ss;
            ss
            << "Can't create a named pipe: "
            << m_signalPipeName;
            throw system_error( ss.str() );
            graceful_quit = 1;
        }

        // Open the pipe for reading
        m_fdSignalPipe = open( m_signalPipeName.c_str(), O_RDWR | O_NONBLOCK );
        if (( -1 == m_fdSignalPipe ) && ( EEXIST != errno ) )
        {
            ostringstream ss;
            ss
            << "Can't open a named pipe: "
            << m_signalPipeName;
            throw system_error( ss.str() );
            graceful_quit = 1;
        }
    }

//=============================================================================
    CAgentBase::~CAgentBase()
    {
        close( m_fdSignalPipe );
        // deleting proof configuration file
        if ( !m_commonOptions.m_proofCFG.empty() )
            unlink( m_commonOptions.m_proofCFG.c_str() );

        unlink( m_signalPipeName.c_str() );
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
    void CAgentBase::Start()
    {
        m_idleWatch.touch();

        // start a monitoring job
        log( LOG_SEVERITY_INFO, "starting a monitor" );
        boost::thread monitorThread( boost::bind( &CAgentBase::monitor, this ) );

        // start the main job and main select loops for servers/workers
        run();
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
//=============================================================================
    void CAgentBase::updateIdle()
    {
        // the current strategy:
        // if at least one status file has an active status,
        // we consider that the whole machine is not idle.
        // The whole machine - I mean all workers/servers assigned to that admin path (xrootd)
        try
        {
            m_proofStatus.enumStatusFiles();
            ProofStatusContainer_t status( m_proofStatus.getStatus() );
            ProofStatusContainer_t::const_iterator iter = status.begin();
            ProofStatusContainer_t::const_iterator iter_end = status.end();
            for ( ; iter != iter_end; ++iter )
            {
                if ( proofstatus_idle != *iter )
                    m_idleWatch.touch();
            }
        }
        catch ( const exception &_e )
        {
            // protect from mass logging
            static uint16_t repeat( 0 );
            ++repeat;
            if ( repeat < 5 )
                log( LOG_SEVERITY_INFO, _e.what() );
        }
    }
}
