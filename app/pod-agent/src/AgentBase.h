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

        Copyright (c) 2009-2011 GSI, Scientific Computing division. All rights reserved.
*************************************************************************/
#ifndef AGENTBASE_H_
#define AGENTBASE_H_
// MiscCommon
#include "Log.h"
// PROOFAgent
#include "Options.h"
#include "IdleWatch.h"
#include "ProofStatusFile.h"

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
            void Start();
            virtual EAgentMode_t GetMode() const = 0;
            bool IsPROOFReady() const;

        protected:
            virtual void run() = 0;
            // a monitoring thread
            // used as a cron process for pod
            virtual void monitor() = 0;
            virtual void log( MiscCommon::LOG_SEVERITY _Severity, const std::string &_msg ) = 0;
            void updateIdle();
            std::string getPROOFCfg();

        protected:
            const PoD::SCommonOptions_t &m_commonOptions;
            CIdleWatch m_idleWatch;
            int m_fdSignalPipe;
            std::string m_signalPipeName;
            CProofStatusFile m_proofStatus;
            unsigned int m_xpdPort;
            pid_t m_xpdPid;
    };

}

#endif /* AGENTBASE_H_ */
