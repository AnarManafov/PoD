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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H

// API
#include <signal.h>
// BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
// MiscCommon
#include "LogImp.h"
//#include "IXMLPersist.h"
#include "SysHelper.h"
#include "CustomIterator.h"
#include "PARes.h"
#include "Process.h"
// PROOFAgent
#include "PacketForwarder.h"
#include "PROOFCfgImpl.h"
#include "PFContainer.h"

namespace PROOFAgent
{
    /**
     *
     * @brief declaration of a signal handler
     *
     */
    void signal_handler( int _SignalNumber );
    /**
     *
     * @brief A base class for PROOFAgent modes - agents.
     *
     */
    class CAgentBase
    {
        public:
            CAgentBase()
            {
                // Registering signals handlers
                struct sigaction sa;
                ::sigemptyset( &sa.sa_mask );
                sa.sa_flags = 0;

                // Register the handler for SIGINT.
                sa.sa_handler = signal_handler;
                ::sigaction( SIGINT, &sa, 0 );
                // Register the handler for SIGTERM
                sa.sa_handler = signal_handler;
                ::sigaction( SIGTERM, &sa, 0 );
            }
            virtual ~CAgentBase()
            {
                // deleting proof configuration file
                if ( !m_sPROOFCfg.empty() )
                    ::unlink( m_sPROOFCfg.c_str() );
            }

        public:
            virtual void Init( /*xercesc::DOMNode* _element*/ )
            {
                _Init( /*_element*/ );
            }
            MiscCommon::ERRORCODE Start( const std::string &_PROOFCfg )
            {
                m_sPROOFCfg = _PROOFCfg;
                boost::thread thrd( boost::bind( &CAgentBase::ThreadWorker, this ) );
                thrd.join();
                return MiscCommon::erOK;
            }
            virtual EAgentMode_t GetMode() const = 0;
            bool IsPROOFReady( unsigned short _Port ) const
            {
                // #1  - Check whether xrootd process exists
                pid_t pid = MiscCommon::getprocbyname( "xrootd" );
                if ( !MiscCommon::IsProcessExist( pid ) )
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
            virtual void _Init( /*xercesc::DOMNode* _element */) = 0;

        protected:
            std::string m_sPROOFCfg;
    };
    /**
     *
     * @brief Agent's data structure (for server mode).
     *
     */
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
    /**
     *
     * @brief Agent's data structure (for client mode).
     *
     */
    typedef struct SAgentClientData
    {
        SAgentClientData() :
                m_nServerPort( 0 ),
                m_nLocalClientPort( 0 )
        {}
        unsigned short m_nServerPort;       //!< PROOFAgent's server port
        std::string m_strServerHost;        //!< PROOFAgent's server host
        unsigned short m_nLocalClientPort;  //!< PROOF's local port (on worker nodes)
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
    /**
     *
     * @brief An agent class, for the server mode of PROOFAgent
     *
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                //MiscCommon::IXMLPersistImpl<CAgentServer>,
                protected CPROOFCfgImpl<CAgentServer>

    {
        public:
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE( "AgentServer" )
        //    DECLARE_XMLPERSIST_IMPL( CAgentServer )

        public:
//            BEGIN_READ_XML_NODE( CAgentServer, "agent_server" )
//            READ_NODE_VALUE( "listen_port", m_Data.m_nPort )
//            READ_NODE_VALUE( "local_client_port_min", m_Data.m_nLocalClientPortMin )
//            READ_NODE_VALUE( "local_client_port_max", m_Data.m_nLocalClientPortMax )
//            END_READ_XML_NODE
//
//            BEGIN_WRITE_XML_CFG( CAgentServer )
//            END_WRITE_XML_CFG

            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }

            void AddPF( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort, const std::string &_sPROOFCfgString )
            {
                boost::mutex::scoped_lock lock( m_PFList_mutex );
                m_PFList.add( _ClientSocket, _nNewLocalPort, _sPROOFCfgString );
            }

            void CleanDisconnectsPF( const std::string &_sPROOFCfg )
            {
                m_PFList.clean_disconnects( _sPROOFCfg );
            }

        protected:
            void ThreadWorker();
            void _Init( /*xercesc::DOMNode* _element*/ )
            {
               // Read( _element );
                InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
            }

        private:
            AgentServerData_t m_Data;
            CPFContainer m_PFList;
            boost::mutex m_PFList_mutex;
    };
    /**
     *
     * @brief An agent class, for the client mode of PROOFAgent
     *
     */
    class CAgentClient:
                public CAgentBase,
                MiscCommon::CLogImp<CAgentClient>,
                //MiscCommon::IXMLPersistImpl<CAgentClient>,
                protected CPROOFCfgImpl<CAgentClient>
    {
        public:
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( "AgentClient" )
 //           DECLARE_XMLPERSIST_IMPL( CAgentClient )

        public:
//            BEGIN_READ_XML_NODE( CAgentClient, "agent_client" )
//            READ_NODE_VALUE( "server_port", m_Data.m_nServerPort )
//            READ_NODE_VALUE( "server_addr", m_Data.m_strServerHost )
//            READ_NODE_VALUE( "local_proofd_port", m_Data.m_nLocalClientPort )
//            END_READ_XML_NODE
//
//            BEGIN_WRITE_XML_CFG( CAgentClient )
//            END_WRITE_XML_CFG

            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void ThreadWorker();
            void _Init( /*xercesc::DOMNode* _element*/ )
            {
              //  Read( _element );
                InfoLog( MiscCommon::erOK, "Agent Client configuration:" ) << m_Data;
            }

        private:
            AgentClientData_t m_Data;
    };

}

#endif
