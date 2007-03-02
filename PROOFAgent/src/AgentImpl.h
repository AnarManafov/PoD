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
            smart_socket( int _domain, int _type, int _protocol )
            {
                m_Socket = socket( _domain, _type, _protocol );
            }
            ~smart_socket()
            {
                if ( m_Socket )
                    close( m_Socket );
            }
            Socket_t* operator&()
            {
                return & m_Socket;
            }

        private:
            Socket_t m_Socket;
    };

    class CAgentBase
    {
        public:
            CAgentBase() : Mode( Unknown )
            {}
            virtual ERRORCODE Init( xercesc::DOMNode* _element ) = 0;
            virtual ERRORCODE Start() = 0;
            virtual ERRORCODE Stop() = 0;

        public:
            const EAgentMode_t Mode;
    };

    typedef struct SAgentServerData
    {
        SAgentServerData() : m_Port( 0 )
        {}
        unsigned short m_Port;
    }
    AgentServerData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentServerData_t &_data )
    {
        _stream
        << "Listen on Port: " << _data.m_Port << std::endl;
        return _stream;
    }

    class CAgentServer :
                public CAgentBase,
                glite_api_wrapper::CLogImp<CAgentServer>,
                IXMLPersist
    {
        public:
            CAgentServer() : Mode( Server )
            {}
            virtual ~CAgentServer()
            {}
            REGISTER_LOG_MODULE(AgentServer)

        public:
            ERRORCODE Init( xercesc::DOMNode* _element );
            ERRORCODE Read( xercesc::DOMNode* _element );
            ERRORCODE Write( xercesc::DOMNode* _element );
            ERRORCODE Start();
            ERRORCODE Stop();


        public:
            const EAgentMode_t Mode;
            AgentServerData_t m_Data;
    };

    class CAgentClient: public CAgentBase
    {
        public:
            CAgentClient() : Mode( Client )
            {}
            ERRORCODE Init( xercesc::DOMNode* _element )
            {
                return erNotImpl;
            }
            ERRORCODE Start()
            {
                return erNotImpl;
            }
            ERRORCODE Stop()
            {
                return erNotImpl;
            }

        public:
            const EAgentMode_t Mode;
    };

}

#endif
