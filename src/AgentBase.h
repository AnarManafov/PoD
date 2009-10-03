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
// MiscCommon
#include "ErrorCode.h"
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
            CAgentBase( const PoD::SCommonOptions_t &_common );
            virtual ~CAgentBase();

        public:
            MiscCommon::ERRORCODE Start();
            virtual EAgentMode_t GetMode() const = 0;
            bool IsPROOFReady( unsigned short _Port ) const;

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
