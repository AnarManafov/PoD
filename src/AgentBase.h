/************************************************************************/
/**
 * @file AgentBase.h
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTBASE_H_
#define AGENTBASE_H_
// API
#include <signal.h>
// BOOST
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
// MiscCommon
#include "Process.h"
#include "INet.h"
// PROOFAgent
#include "Options.h"

namespace PROOFAgent
{
//=============================================================================
    /**
     *
     * @brief declaration of a signal handler
     *
     */
    void signal_handler( int _SignalNumber );

//=============================================================================
    /**
     *
     * @brief A base class for PROOFAgent modes - agents.
     *
     */
    class CAgentBase
    {
        public:
            CAgentBase( const PoD::SCommonOptions_t &_common ):
                    m_commonOptions( _common ),
                    m_agentServerListenPort( 0 )
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
            virtual ~CAgentBase()
            {
                // deleting proof configuration file
                if ( !m_commonOptions.m_proofCFG.empty() )
                    ::unlink( m_commonOptions.m_proofCFG.c_str() );
            }

        public:
            MiscCommon::ERRORCODE Start()
            {
                boost::thread thrd( boost::bind( &CAgentBase::ThreadWorker, this ) );
                thrd.join();
                return MiscCommon::erOK;
            }
            virtual EAgentMode_t GetMode() const = 0;
            bool IsPROOFReady( unsigned short _Port ) const
            {
                // #1  - Check whether xrootd process exists
                MiscCommon::vectorPid_t pids = MiscCommon::getprocbyname( "xrootd" );
                if ( pids.empty() )
                    return false;

                MiscCommon::vectorPid_t::const_iterator iter = pids.begin();
                MiscCommon::vectorPid_t::const_iterator iter_end = pids.end();
                // checking that the process is running under current's user id
                bool found = false;
                for ( ; iter != iter_end; ++iter )
                {
                    MiscCommon::CProcStatus p;
                    p.Open( *iter );
                    std::istringstream ss( p.GetValue( "Uid" ) );
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

        protected:
            virtual void ThreadWorker() = 0;
            void readServerInfoFile( const std::string &_filename );

        protected:
            const PoD::SCommonOptions_t &m_commonOptions;
            unsigned int m_agentServerListenPort;
            std::string m_agentServerHost;
    };

}

#endif /* AGENTBASE_H_ */
