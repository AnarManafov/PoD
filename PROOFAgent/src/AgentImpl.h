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


    /** @class CAgentServer
     *  @brief
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                protected CPROOFCfgImpl<CAgentServer>
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

            void CleanDisconnectsPF( const std::string &_sPROOFCfg )
            {
                m_PFList.clean_disconnects(_sPROOFCfg);
            }

        protected:
            void ThreadWorker();

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
                protected CPROOFCfgImpl<CAgentClient>
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
            AgentClientData_t m_Data;
    };

}

#endif
