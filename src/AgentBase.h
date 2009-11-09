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
// BOOST
#include <boost/thread/thread.hpp>
// MiscCommon
#include "ErrorCode.h"
#include "Log.h"
// PROOFAgent
#include "Options.h"
#include "IdleWatch.h"

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
            CAgentBase( const PoD::SCommonOptions_t &_common );
            virtual ~CAgentBase();

        public:
            MiscCommon::ERRORCODE Start();
            virtual EAgentMode_t GetMode() const = 0;
            bool IsPROOFReady( unsigned short _Port ) const;

        protected:
            virtual void run() = 0;
            virtual void monitor() = 0;
            virtual void log( MiscCommon::LOG_SEVERITY _Severity, const std::string &_msg ) = 0;
            void readServerInfoFile( const std::string &_filename );

        protected:
            const PoD::SCommonOptions_t &m_commonOptions;
            unsigned int m_agentServerListenPort;
            std::string m_agentServerHost;
            boost::thread *m_monitorThread;
            CIdleWatch m_idleWatch;
            int m_fdSignalPipe;
            std::string m_signalPipeName;
    };

}

#endif /* AGENTBASE_H_ */
