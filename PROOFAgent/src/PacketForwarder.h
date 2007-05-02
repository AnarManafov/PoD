/************************************************************************/
/**
 * @file PacketForwarder.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                     2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENTPACKETFORWARDER_H
#define PROOFAGENTPACKETFORWARDER_H

// Our
#include "INet.h"
#include "LogImp.h"
#include "def.h"
#include "MiscUtils.h"
#include "BOOSTHelper.h"

namespace PROOFAgent
{    

    class CPacketForwarder:
                public MiscCommon::CLogImp<CPacketForwarder>,
                MiscCommon::NONCopyable  //HACK: Since smart_socket doesn't support copy-constructor and ref. count
    {
        public:
            CPacketForwarder( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort ) :
                    m_ClientSocket( _ClientSocket ),
                    m_nPort( _nNewLocalPort )
            {}

            ~CPacketForwarder()
            {}

            REGISTER_LOG_MODULE( PacketForwarder );

            void WriteBuffer( MiscCommon::BYTEVector_t &_Buf, MiscCommon::INet::smart_socket &_socket ) throw ( std::exception );

        public:
            MiscCommon::ERRORCODE Start( bool _Join = false );

        protected:
            void ThreadWorker( MiscCommon::INet::smart_socket *_SrvSocket, MiscCommon::INet::smart_socket *_CltSocket );

        private:
            MiscCommon::ERRORCODE _Start( bool _Join );
            void ReportPackage( MiscCommon::INet::Socket_t _socket, MiscCommon::BYTEVector_t &_buf, bool _received = true /*weather package received or submitted*/ )
            {
                std::string strSocketInfo;
                MiscCommon::INet::socket2string( _socket, &strSocketInfo );
                std::string strSocketPeerInfo;
                MiscCommon::INet::peer2string( _socket, &strSocketPeerInfo );
                std::stringstream ss;
                ss
                << ( _received ? "RECEIVED: " : "FORWARDED: " )
                << ( _received ? strSocketPeerInfo : strSocketInfo ) << " |-> " << ( _received ? strSocketInfo : strSocketPeerInfo ) << "\n"                
                << std::string( reinterpret_cast<char*>( &_buf[ 0 ] ) ) << "\n";                
                DebugLog( MiscCommon::erOK, ss.str() );
            }

        private:
            MiscCommon::INet::smart_socket m_ClientSocket;
            MiscCommon::INet::smart_socket m_ServerCocket;
            unsigned short m_nPort;
            MiscCommon::Thread_PTR_t m_thrd_clnt;
            MiscCommon::Thread_PTR_t m_thrd_srv;
            MiscCommon::Thread_PTR_t m_thrd_serversocket;
    };

}

#endif
