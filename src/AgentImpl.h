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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H

// API
#include <signal.h>
// BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// MiscCommon
#include "LogImp.h"
#include "SysHelper.h"
#include "CustomIterator.h"
#include "PARes.h"
#include "Process.h"
// PROOFAgent
#include "PacketForwarder.h"
#include "PROOFCfgImpl.h"
#include "PFContainer.h"
//=============================================================================
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

        protected:
            std::string m_sPROOFCfg;
    };
    //=============================================================================
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
    //=============================================================================
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
    //=============================================================================
    inline std::ostream &operator <<( std::ostream &_stream, const AgentClientData_t &_data )
    {
        _stream
        << "server info: [" << _data.m_strServerHost << ":" << _data.m_nServerPort << "];"
        << "\n"
        << "local listen port: " << _data.m_nLocalClientPort
        << std::endl;
        return _stream;
    }
    //=============================================================================
    /**
     *
     * @brief An agent class, for the server mode of PROOFAgent
     *
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                protected CPROOFCfgImpl<CAgentServer>

    {
        public:
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE( "AgentServer" )

        public:
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

        private:
            template<class Archive>
            void save( Archive & _ar, const unsigned int /*_version*/ ) const
            {
                _ar
                & BOOST_SERIALIZATION_NVP( m_Data.m_nPort )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPortMin )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPortMax );
            }
            template<class Archive>
            void load( Archive & _ar, const unsigned int /*_version*/ )
            {
                _ar
                & BOOST_SERIALIZATION_NVP( m_Data.m_nPort )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPortMin )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPortMax );

                InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
            }
            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            AgentServerData_t m_Data;
            CPFContainer m_PFList;
            boost::mutex m_PFList_mutex;
    };
    //=============================================================================
    /**
     *
     * @brief An agent class, for the client mode of PROOFAgent
     *
     */
    class CAgentClient:
                public CAgentBase,
                MiscCommon::CLogImp<CAgentClient>,
                protected CPROOFCfgImpl<CAgentClient>
    {
        public:
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( "AgentClient" )

        public:
            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void ThreadWorker();

        private:
            template<class Archive>
            void save( Archive & _ar, const unsigned int /*_version*/ ) const
            {
                _ar
                & BOOST_SERIALIZATION_NVP( m_Data.m_nServerPort )
                & BOOST_SERIALIZATION_NVP( m_Data.m_strServerHost )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPort );
            }
            template<class Archive>
            void load( Archive & _ar, const unsigned int /*_version*/ )
            {
                _ar
                & BOOST_SERIALIZATION_NVP( m_Data.m_nServerPort )
                & BOOST_SERIALIZATION_NVP( m_Data.m_strServerHost )
                & BOOST_SERIALIZATION_NVP( m_Data.m_nLocalClientPort );

                InfoLog( MiscCommon::erOK, "Agent Client configuration:" ) << m_Data;
            }
            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            AgentClientData_t m_Data;
    };

}


BOOST_CLASS_VERSION( PROOFAgent::CAgentServer, 1 )
BOOST_CLASS_VERSION( PROOFAgent::CAgentClient, 1 )

#endif
