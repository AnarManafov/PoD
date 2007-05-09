/************************************************************************/
/**
 * @file AgentImpl.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
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

// PROOFAgent
#include "ErrorCode.h"
#include "LogImp.h"
#include "IXMLPersist.h"
#include "PacketForwarder.h"
#include "SysHelper.h"

/**
 * @brief A general name space for PROOFAgent application 
 **/
namespace PROOFAgent
{
    typedef enum{ Unknown, Server, Client }EAgentMode_t;

    // declaration of signal handler
    void signal_handler( int _SignalNumber );

    template <class _T>
    struct CPROOFCfgImp
    {
        void CreatePROOFCfg( const std::string &_PROOFCfg )
        {
            std::ofstream f_out( _PROOFCfg.c_str() );
            // TODO: check file-errors
            _T *pThis = reinterpret_cast<_T*>( this );

            // getting local host name
            std::string host;
            MiscCommon::get_hostname( &host );
            // master host name is the same for Server and Worker and equal to local host name
            f_out << "master " << host << std::endl;

            if ( pThis->GetMode() == Client )
            {
                f_out << "worker " << host << " perf=100" << std::endl;
            }
        }
        void AddWrk2PROOFCfg( const std::string &_PROOFCfg, const std::string &_UsrName, unsigned short _Port )
        {
            _T *pThis = reinterpret_cast<_T*>( this );
            if ( pThis->GetMode() != Server )
                return ;

            std::ofstream f_out( _PROOFCfg.c_str(), std::ios_base::out | std::ios_base::app );
            if ( !f_out.is_open() )
                throw std::runtime_error("Can't open the PROOF configuration file: " + _PROOFCfg );

            f_out << "worker " << _UsrName << "@localhost:" << _Port << " perf=100 workdir=~/" << std::endl;
        }
    };

    /**
      *  @brief
     */
    class CAgentBase:
                MiscCommon::IXMLPersist
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
            {}

        public:
            virtual MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element )
            {
                return this->Read( _element );
            }
            MiscCommon::ERRORCODE Start( const std::string &_PROOFCfgDir )
            {
                boost::thread thrd( boost::bind( &CAgentBase::ThreadWorker, this, _PROOFCfgDir ) );
                thrd.join();
                return MiscCommon::erOK;
            }
            virtual EAgentMode_t GetMode() const = 0;

        protected:
            virtual void ThreadWorker( const std::string &_PROOFCfg ) = 0;
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

    template <class _T>
    struct SDelete
    {
        bool operator() ( _T *_val )
        {
            if ( _val )
                delete _val;
            return true;
        }
    };

    class PF_Container
    {
            typedef CPacketForwarder pf_container_value;
            typedef std::list<pf_container_value *> pf_container_type;

        public:
            PF_Container()
            {}
            ~PF_Container()
            {
                std::for_each( m_container.begin(), m_container.end(), SDelete<pf_container_value>() );
            }
            void add( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort )
            {
                CPacketForwarder * pf = new CPacketForwarder( _ClientSocket, _nNewLocalPort );
                pf->Start();
                m_container.push_back( pf );
            }

        private:
            pf_container_type m_container;

    };

    /** @class CAgentServer
     *  @brief
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                protected CPROOFCfgImp<CAgentServer>
    {
        public:
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE( AgentServer );

        public:
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );
            virtual EAgentMode_t GetMode() const
            {
                return Server;
            }

            void AddPF( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort )
            {
                boost::mutex::scoped_lock lock ( m_PFList_mutex );
                m_PFList.add( _ClientSocket, _nNewLocalPort );
            }

        protected:
            void ThreadWorker( const std::string &_PROOFCfg );

        private:
            //          const EAgentMode_t Mode;
            AgentServerData_t m_Data;
            PF_Container m_PFList;
            boost::mutex m_PFList_mutex;
    };

    /** @class CAgentClient
      *  @brief
      */
    class CAgentClient:
                public CAgentBase,
                MiscCommon::CLogImp<CAgentClient>,
                protected CPROOFCfgImp<CAgentClient>
    {
        public:
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( AgentClient );

        public:
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );
            virtual EAgentMode_t GetMode() const
            {
                return Client;
            }

        protected:
            void ThreadWorker( const std::string &_PROOFCfg );

        private:
            //   const EAgentMode_t Mode;
            AgentClientData_t m_Data;
    };

}

#endif
