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

// XML parser
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>

// PROOFAgent
#include "ErrorCode.h"

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
        enum EMode{Mode = Unknown};
        public:
            virtual ERRORCODE Init( xercesc::DOMNode* _element ) = 0;
            virtual ERRORCODE Start( xercesc::DOMNode* _element ) = 0;
            virtual ERRORCODE Stop( xercesc::DOMNode* _element ) = 0;
            EAgentMode_t GetMode()
            {
                return static_cast<EAgentMode_t>(Mode);
            }
    };

    class CAgentServer : public CAgentBase
    {
        enum EMode{Mode = Server};
        public:
            ERRORCODE _Init()
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

    class CAgentClient: public CAgentBase
    {
        enum EMode{Mode = Client};
        public:
            ERRORCODE _Init()
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
