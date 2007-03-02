/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef AGENTIMPL_H
#define AGENTIMPL_H

// API
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// BOOST
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

// STD
#include <unistd.h>

// PROOFAgent
#include "ErrorCode.h"
#include "LogImp.h"
#include "IXMLPersist.h"

namespace PROOFAgent
{
    typedef int Socket_t;
    typedef enum{ Unknown, Server, Client }EAgentMode_t;


    class smart_socket
    {
        public:
            smart_socket() : m_Socket( 0 )
            {}
            smart_socket( int _Socket ) : m_Socket( _Socket )
            {}
            smart_socket( int _domain, int _type, int _protocol )
            {
                m_Socket = ::socket( _domain, _type, _protocol );
            }
            ~smart_socket()
            {
                close();
            }
            Socket_t* operator&()
            {
                return & m_Socket;
            }
            operator int()
            {
                return static_cast<int>( m_Socket );
            }
            int operator =( const int &_Val )
            {
                close();
                return m_Socket = _Val;
            }
            void close()
            {
                if ( m_Socket )
                    ::close( m_Socket );
            }

        private:
            Socket_t m_Socket;
    };


    /** @class CAgentBase
      *  @brief
     */
    class CAgentBase
    {
        public:
            CAgentBase() : Mode( Unknown )
            {}
            virtual MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element ) = 0;
            virtual MiscCommon::ERRORCODE Start() = 0;

        public:
            const EAgentMode_t Mode;
    };

    typedef struct SAgentServerData
    {
        SAgentServerData() : m_nPort( 0 )
        {}
        unsigned short m_nPort;
    }
    AgentServerData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentServerData_t &_data )
    {
        _stream
        << "Listen on Port: " << _data.m_nPort << std::endl;
        return _stream;
    }

    typedef struct SAgentClientData
    {
        SAgentClientData() : m_nPort( 0 )
        {}
        unsigned short m_nPort;
        std::string m_strHost;
    }
    AgentClientData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentClientData_t &_data )
    {
        _stream
        << "Connecting to Port: " << _data.m_nPort << "\n"
        << "on Host: " << _data.m_strHost << std::endl;
        return _stream;
    }

    /** @class CAgentServer
     *  @brief
     */
    class CAgentServer :
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                MiscCommon::IXMLPersist
    {
        public:
            CAgentServer() :
                    Mode( Server )
            {}
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE( AgentServer )

        public:
            MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Start();
            void LogThread( const std::string _Msg, MiscCommon::ERRORCODE _erCode = MiscCommon::erOK )
            {
                boost::mutex::scoped_lock lock ( m_log_mutex );
                InfoLog( _erCode, _Msg );
            }


        public:
            const EAgentMode_t Mode;
            AgentServerData_t m_Data;
            boost::mutex m_log_mutex;
    };

    /** @class CAgentClient
      *  @brief
      */
    class CAgentClient:
                public CAgentBase,
                MiscCommon::CLogImp<CAgentServer>,
                MiscCommon::IXMLPersist
    {
        public:
            CAgentClient() : Mode( Client )
            {}
            virtual ~CAgentClient()
            {}
            REGISTER_LOG_MODULE( AgentServer )

        public:
            MiscCommon::ERRORCODE Init( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Read( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Write( xercesc::DOMNode* _element );
            MiscCommon::ERRORCODE Start();

        public:
            const EAgentMode_t Mode;
            AgentClientData_t m_Data;
    };

}

#endif
