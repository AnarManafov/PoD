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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
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

// TODO: Move it to config.
const unsigned int g_BUF_SIZE = 5000;

namespace PROOFAgent
{
    class CIdleWatch
    {
        public:
            void touch()
            {
                m_startTime = time( NULL );
            }
            bool isTimedout( int _numSeconds )
            {
                if ( _numSeconds <= 0 )
                    return false;

                time_t curTime = time( NULL );
                return (( curTime - m_startTime ) >= _numSeconds );
            }

        private:
            time_t m_startTime;
    };
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
                    m_shutdownIfIdleForSec( 0 ),
                    m_buf( g_BUF_SIZE ),
                    m_bytesToSend( 0 )
            {}

            ~CPacketForwarder()
            {}

            REGISTER_LOG_MODULE( "PacketForwarder" );

        public:
            MiscCommon::ERRORCODE Start( bool _ClientMode = false, int _shutdownIfIdleForSec = 0 );
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
            bool dealWithData( MiscCommon::INet::smart_socket *_Input, MiscCommon::INet::smart_socket *_Output );
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
            MiscCommon::BOOSTHelper::Thread_PTR_t m_thrd_serversocket;
            boost::mutex m_mutex;
            int m_shutdownIfIdleForSec;
            CIdleWatch m_idleWatch;
            MiscCommon::BYTEVector_t m_buf;
            size_t m_bytesToSend;
    };

}

#endif
