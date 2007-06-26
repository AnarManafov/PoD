/************************************************************************/
/**
 * @file AgentImpl.h
 * @brief Header file of AgentServer and AgentClient
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
#include <functional>

// PROOFAgent
#include "ErrorCode.h"
#include "LogImp.h"
#include "IXMLPersist.h"
#include "PacketForwarder.h"
#include "SysHelper.h"
#include "CustomIterator.h"

namespace PROOFAgent
{
    typedef enum{ Unknown, Server, Client }EAgentMode_t;

    // declaration of signal handler
    void signal_handler( int _SignalNumber );

    /**
     * @brief This class creates proof.conf for server and client
     * @note
     example of proof.conf for server
     @verbatim
      
     master depc218.gsi.de  workdir=~/proof
     worker manafov@localhost:20001 perf=100 workdir=~/
      
     @endverbatim
     example of proof.conf for client
     @verbatim
      
     master lxial24.gsi.de
     worker lxial24.gsi.de perf=100
      
     @endverbatim
     **/
    template <class _T>
    struct CPROOFCfgImp
    {
        void CreatePROOFCfg( const std::string &_PROOFCfg) const
        {
            std::ofstream f_out( _PROOFCfg.c_str() );
            // TODO: check file-errors
            const _T *pThis = reinterpret_cast<const _T*>( this );

            // getting local host name
            std::string host;
            MiscCommon::get_hostname( &host );
            // master host name is the same for Server and Worker and equal to local host name
            f_out << "#master " << host << std::endl;
            f_out << "master " << host << std::endl;

            if ( pThis->GetMode() == Client )
            {
                f_out << "worker " << host << " perf=100" << std::endl;
            }
        }
        void AddWrk2PROOFCfg( const std::string &_PROOFCfg, const std::string &_UsrName,
                              unsigned short _Port, const std::string &_RealWrkHost, std::string *_RetVal = NULL ) const
        {
            const _T * pThis = reinterpret_cast<const _T*>( this );
            if ( pThis->GetMode() != Server )
                return ;

            std::ofstream f_out( _PROOFCfg.c_str(), std::ios_base::out | std::ios_base::app );
            if ( !f_out.is_open() )
                throw std::runtime_error("Can't open the PROOF configuration file: " + _PROOFCfg );

            std::stringstream ss;
            ss << "#worker " << _UsrName << "@" << _RealWrkHost << " (redirect through localhost:" << _Port << ")";

            f_out << ss.str() << std::endl;
            f_out << "worker " << _UsrName << "@localhost:" << _Port << " perf=100" << std::endl;
            if ( _RetVal )
                *_RetVal = ss.str();
        }
        void RemoveEntry( const std::string &_PROOFCfg, const std::string &_sPROOFCfgString ) const
        {
            // Read proof.conf in order to update it
            std::ifstream f( _PROOFCfg.c_str() );
            if ( !f.is_open() )
                return ;

            MiscCommon::StringVector_t vec;

            std::copy(MiscCommon::custom_istream_iterator<std::string>(f),
                      MiscCommon::custom_istream_iterator<std::string>(),
                      std::back_inserter(vec));

            std::ofstream f_out( _PROOFCfg.c_str() );

            MiscCommon::StringVector_t::const_iterator iter = vec.begin();
            MiscCommon::StringVector_t::const_iterator iter_end = vec.end();
            for ( ; iter != iter_end; ++iter )
            {
                if ( *iter == _sPROOFCfgString )
                {
                    ++iter;
                    continue;
                }
                f_out << *iter;
            }
        }

    }
    ;

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
            {
                if ( !m_sPROOFCfg.empty() )
                    ::unlink( m_sPROOFCfg.c_str() );
            }

        public:
            virtual MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element )
            {
                return this->Read( _element );
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

    template <class _T>
    struct SDelete: public std::binary_function<_T, bool, bool>
    {
        bool operator() ( _T _val, bool _DelDisconnects = false ) const
        {
            if ( !_val.first )
                return true;

            if ( !_DelDisconnects )
            {
                delete _val.first;
                _val.first = NULL;
            }
            else if ( !_val.first->IsValid() )
            {
                delete _val.first;
                _val.first = NULL;
            }

            return true;
        }
    };

    class PF_Container
    {
            typedef CPacketForwarder pf_container_value;
            typedef std::pair<pf_container_value *, std::string> container_value;
            typedef std::list<container_value> pf_container_type;

        public:
            PF_Container()
            {}
            ~PF_Container()
            {
                std::for_each( m_container.begin(), m_container.end(), SDelete<container_value>() );
            }
            void add( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort, const std::string &_sPROOFCfgString )
            {
                CPacketForwarder * pf = new CPacketForwarder( _ClientSocket, _nNewLocalPort );
                pf->Start();
                m_container.push_back( std::make_pair(pf, _sPROOFCfgString) );
            }
            void clean_disconnects()
            {
                std::for_each( m_container.begin(), m_container.end(), std::bind2nd(SDelete<container_value>(), true) );
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

            void AddPF( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort, const std::string &_sPROOFCfgString)
            {
                boost::mutex::scoped_lock lock ( m_PFList_mutex );
                m_PFList.add( _ClientSocket, _nNewLocalPort, _sPROOFCfgString);
            }

            void CleanDisconnectsPF()
            {
                m_PFList.clean_disconnects();
            }

        protected:
            void ThreadWorker();

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
            void ThreadWorker();

        private:
            //   const EAgentMode_t Mode;
            AgentClientData_t m_Data;
    };

}

#endif
