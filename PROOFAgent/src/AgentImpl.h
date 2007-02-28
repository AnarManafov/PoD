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

namespace proofagent
{
    typedef int Socket_t;
    
    class smart_socket
    {
        public:
            smart_socket() : m_Socket( 0 )
            {}
            smart_socket(int _domain, int _type, int _protocol)
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
                return &m_Socket;
            }

        private:
            Socket_t m_Socket;
    };

    template <typename _T>
    class CAgentImpl
    {
        public:
            ERRORCODE Init()
            {
                _T * pT = static_cast<_T*>( this );
                return pT->ReadConfig();
            }
            ERRORCODE Start()
            {
                _T * pT = static_cast<_T*>( this );
                return pT->_Start();
            }
            ERRORCODE Stop()
            {
                _T * pT = static_cast<_T*>( this );
                return pT->_Stop();
            }
    };

    class CAgentServer: public CAgentImpl<CAgentServer>
    {
            friend class CAgentImpl<CAgentServer>;
        private:
            ERRORCODE ReadConfig()
            {
                return erOK;
            }
            ERRORCODE _Start()
            {
                return erNotImpl;
            }
            ERRORCODE _Stop()
            {
                return erNotImpl;
            }
    };

    class CAgentClient: public CAgentImpl<CAgentClient>
    {
            friend class CAgentImpl<CAgentClient>;
        private:
            ERRORCODE ReadConfig()
            {
                return erNotImpl;
            }
            ERRORCODE _Start()
            {
                return erNotImpl;
            }
            ERRORCODE _Stop()
            {
                return erNotImpl;
            }
    };

}

#endif
