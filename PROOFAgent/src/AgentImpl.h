/************************************************************************/
/**
 * @file AgentImpl.h
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H

// API
#include <signal.h>

// BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// STD
#include <list>
#include <functional>

// PROOFAgent
//#include "ErrorCode.h"
#include "LogImp.h"
#include "IXMLPersist.h"
#include "PacketForwarder.h"
#include "SysHelper.h"
#include "CustomIterator.h"
#include "PARes.h"
#include "PROOFCfgImpl.h"
#include "PFContainer.h"

namespace PROOFAgent
{

    // declaration of signal handler
    void signal_handler( int _SignalNumber );

    /**
      *  @brief
     */
    class CAgentBase
    {
        public:
            CAgentBase()
            {
                struct sigaction sa;
                ::sigemptyset (&sa.sa_mask);
                sa.sa_flags = 0;

                // Register the handler for SIGINT.
                sa.sa_handler = signal_handler;
                ::sigaction (SIGINT, &sa, 0);
                // Register the handler for SIGTERM
                sa.sa_handler = signal_handler;
                ::sigaction (SIGTERM, &sa, 0);
            }
            virtual ~CAgentBase()
            {
                if ( !m_sPROOFCfg.empty() )
                    ::unlink( m_sPROOFCfg.c_str() );
            }

        public:
            virtual void Init( xercesc::DOMNode* _element )
            {
                _Init( _element );
            }
            MiscCommon::ERRORCODE Start( const std::string &_PROOFCfg )
            {
                m_sPROOFCfg = _PROOFCfg;
                boost::thread thrd( boost::bind( &CAgentBase::ThreadWorker, this ) );
                thrd.join();
                return MiscCommon::erOK;
            }
            virtual EAgentMode_t GetMode() const = 0;

        protected:
            virtual void ThreadWorker() = 0;
            virtual void _Init( xercesc::DOMNode* _element ) = 0;

        protected:
            std::string m_sPROOFCfg;
    };

    typedef struct SAgentServerData
    {
        SAgentServerData() :
                m_nPort( 0 ),
                m_nLocalClientPortMin( 0 ),
                m_nLocalClientPortMax( 0 )
        {}
        unsigned short m_nPort;
        unsigned short m_nLocalClientPortMin;
        unsigned short m_nLocalClientPortMax;
    }
    AgentServerData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentServerData_t &_data )
    {
        _stream
        << "Listen on Port: " << _data.m_nPort << "\n"
        << "a Local Clients Ports: " << _data.m_nLocalClientPortMin << "-" << _data.m_nLocalClientPortMax << std::endl;
        return _stream;
    }

    typedef struct SAgentClientData
    {
        SAgentClientData() :
                m_nServerPort( 0 ),
                m_nLocalClientPort( 0 )
        {}
        unsigned short m_nServerPort;
        std::string m_strServerHost;
        unsigned short m_nLocalClientPort;
    }
    AgentClientData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentClientData_t &_data )
    {
        _stream
        << "server info: [" << _data.m_strServerHost << ":" << _data.m_nServerPort << "];"
        << "\n"
        << "local listen port: " << _data.m_nLocalClientPort
        << std::endl;
        return _stream;
    }


    /** @class CAgentServer
     *  @brief
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                MiscCommon::IXMLPersistImpl<CAgentServer>,
                protected CPROOFCfgImpl<CAgentServer>

    {
        public:
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE( "AgentServer" )
            DECLARE_XMLPERSIST_IMPL(CAgentServer)

        public:
            BEGIN_READ_XML_NODE(CAgentServer, "agent_server")
            READ_ELEMENT( "listen_port", m_Data.m_nPort )
            READ_ELEMENT( "local_client_port_min", m_Data.m_nLocalClientPortMin )
            READ_ELEMENT( "local_client_port_max", m_Data.m_nLocalClientPortMax )
            END_READ_XML_NODE

            BEGIN_WRITE_XML_CFG(CAgentServer)
            END_WRITE_XML_CFG

            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }

            void AddPF( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort, const std::string &_sPROOFCfgString)
            {
                boost::mutex::scoped_lock lock ( m_PFList_mutex );
                m_PFList.add( _ClientSocket, _nNewLocalPort, _sPROOFCfgString);
            }

            void CleanDisconnectsPF( const std::string &_sPROOFCfg )
            {
                m_PFList.clean_disconnects(_sPROOFCfg);
            }

        protected:
            void ThreadWorker();
            void _Init( xercesc::DOMNode* _element )
            {
                Read( _element );
                InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
            }

        private:
            AgentServerData_t m_Data;
            CPFContainer m_PFList;
            boost::mutex m_PFList_mutex;
    };

    /** @class CAgentClient
      *  @brief
      */
    class CAgentClient:
                public CAgentBase,
                MiscCommon::CLogImp<CAgentClient>,
                MiscCommon::IXMLPersistImpl<CAgentClient>,
                protected CPROOFCfgImpl<CAgentClient>
    {
        public:
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( "AgentClient" )
            DECLARE_XMLPERSIST_IMPL(CAgentClient)

        public:
            BEGIN_READ_XML_NODE(CAgentClient, "agent_client")
            READ_ELEMENT( "server_port", m_Data.m_nServerPort )
            READ_ELEMENT( "server_addr", m_Data.m_strServerHost )
            READ_ELEMENT( "local_proofd_port", m_Data.m_nLocalClientPort )
            END_READ_XML_NODE

            BEGIN_WRITE_XML_CFG(CAgentClient)
            END_WRITE_XML_CFG

            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void ThreadWorker();
            void _Init( xercesc::DOMNode* _element )
            {
                Read( _element );
                InfoLog( MiscCommon::erOK, "Agent Client configuration:" ) << m_Data;
            }

        private:
            AgentClientData_t m_Data;
    };

}

#endif
