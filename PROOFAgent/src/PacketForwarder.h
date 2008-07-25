/************************************************************************/
/**
 * @file PacketForwarder.h
 * @brief Packet Forwarder's header file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENTPACKETFORWARDER_H
#define PROOFAGENTPACKETFORWARDER_H

// STD
#include <iomanip>
// MiscCommon
#include "INet.h"
#include "LogImp.h"
#include "BOOSTHelper.h"
#include "HexView.h"

namespace PROOFAgent
{
    /**
     *
     * @brief The CPacketForwarder class, creates a proxy between client sockets and server's socket given by a port number.
     * @brief It tries to connect by the given port number and then redirects all traffic from/to the client.
     *
     */
    class CPacketForwarder:
                public MiscCommon::CLogImp<CPacketForwarder>,
                MiscCommon::NONCopyable  //HACK: Since smart_socket doesn't support copy-constructor and ref. count
    {
        public:
            CPacketForwarder( MiscCommon::INet::Socket_t _ClientSocket, unsigned short _nNewLocalPort ) :
                    m_ClientSocket( _ClientSocket ),
                    m_nPort( _nNewLocalPort ),
                    m_Counter( 0 )
            {}

            ~CPacketForwarder()
            {}

            REGISTER_LOG_MODULE( "PacketForwarder" );

        public:
            MiscCommon::ERRORCODE Start( bool _ClientMode = false );
            bool IsValid() const
            {
                return ( m_ClientSocket.is_valid() || m_ServerSocket.is_valid() );
            }

        protected:
            void ThreadWorker( MiscCommon::INet::smart_socket *_SrvSocket, MiscCommon::INet::smart_socket *_CltSocket );

        private:
            MiscCommon::ERRORCODE _Start( bool _ClientMode );
            void SpawnServerMode();
            void SpawnClientMode();
            bool ForwardBuf( MiscCommon::INet::smart_socket *_Input, MiscCommon::INet::smart_socket *_Output );
            void ReportPackage( MiscCommon::INet::Socket_t _socket1, MiscCommon::INet::Socket_t _socket2,
                                const MiscCommon::BYTEVector_t &_buf )
            {
                std::string strSocket1;
                MiscCommon::INet::socket2string( _socket1, &strSocket1 );
                std::string strSocket2;
                MiscCommon::INet::socket2string( _socket2, &strSocket2 );

                std::stringstream ss;
                ss
                << strSocket1 << " > " << strSocket2
                << " (" << _buf.size() << " bytes): ";
                if ( !_buf.empty() )
                {
                    ss
                    << "\n"
                    << MiscCommon::BYTEVectorHexView_t( _buf )
                    << "\n";
                }
                DebugLog( MiscCommon::erOK, ss.str() );
            }

        private:
            MiscCommon::INet::smart_socket m_ClientSocket;
            MiscCommon::INet::smart_socket m_ServerSocket;
            unsigned short m_nPort;
            MiscCommon::BOOSTHelper::Thread_PTR_t m_thrd_clnt;
            MiscCommon::BOOSTHelper::Thread_PTR_t m_thrd_srv;
            MiscCommon::BOOSTHelper::Thread_PTR_t m_thrd_serversocket;
            boost::mutex m_mutex;
            // TODO: remove this member whenever a boost::thread::joinable() will be accessible (gLite UI 3.1 it is not)
            // This is a workaround in order to properly close sockets in PF in server mode.
            unsigned short m_Counter;
    };

}

#endif
