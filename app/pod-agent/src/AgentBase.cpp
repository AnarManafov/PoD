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

// FIX: silence a warning until BOOST fix it
// boost/thread/pthread/condition_variable.hpp:53:19: warning: unused variable 'res' [-Wunused-variable]
// clang
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
// gcc
#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
// push doesn't work for gcc 4.2
//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include <boost/thread/thread.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
//#pragma GCC diagnostic pop
#pragma GCC diagnostic warning "-Wunused-variable"
#endif

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
    // memberof to silence doxygen warning:
    // warning: no matching class member found for
    // This happens because doxygen is not handling namespaces in arguments properly
    /**
     * @memberof CAgentBase
     *
     */
    CAgentBase::CAgentBase( const SCommonOptions_t &_common ) :
        m_commonOptions( _common ),
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
        if(( -1 == ret_val ) && ( EEXIST != errno ) )
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
        if(( -1 == m_fdSignalPipe ) && ( EEXIST != errno ) )
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
        string proofCfg( getPROOFCfg() );
        if( !proofCfg.empty() )
            unlink( proofCfg.c_str() );

        unlink( m_signalPipeName.c_str() );
    }
//=============================================================================
    string CAgentBase::getPROOFCfg()
    {
        string proofCfg( m_commonOptions.m_workDir );
        smart_append( &proofCfg, '/' );
        proofCfg += "proof.conf";
        return proofCfg;
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
        // #1  - Check whether xproofd process exists
        try
        {
            vectorPid_t pids = getprocbyname( "xproofd", true );
            if( pids.empty() )
                return false;
        }
        catch( ... )
        {
            return false;
        }

        // #2 - Check whether PROOF port is in use
        if( 0 == _Port )
            return true;
        if( 0 != MiscCommon::INet::get_free_port( _Port ) )
            return false; // PROOF port is not in use, that means we can't connect to it
        // TODO: Implement more checks of xproofd here
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
            for( ; iter != iter_end; ++iter )
            {
                if( proofstatus_idle != *iter )
                {
                    m_idleWatch.touch();
                    log( LOG_SEVERITY_DEBUG, "updateIdle: touch" );
                }
                else
                {
                    log( LOG_SEVERITY_DEBUG, "updateIdle: idle" );
                }
            }
        }
        catch( const exception &_e )
        {
            log( LOG_SEVERITY_WARNING, _e.what() );
        }
    }
}
