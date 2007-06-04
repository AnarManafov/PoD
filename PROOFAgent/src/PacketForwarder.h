/************************************************************************/
/**
 * @file PacketForwarder.h
 * @brief Packet Forwarder's header file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENTPACKETFORWARDER_H
#define PROOFAGENTPACKETFORWARDER_H

// STD
#include <iomanip>

// Our
#include "INet.h"
#include "LogImp.h"
#include "def.h"
#include "MiscUtils.h"
#include "BOOSTHelper.h"

namespace PROOFAgent
{
    template < class _T >
    struct _SHexInserter
    {
        _SHexInserter( std::ostream &_stream ): m_stream(_stream)
        {
            m_stream << std::hex << std::uppercase;
        }
        bool operator()( typename _T::value_type _Val )
        {
            m_stream << std::setw(2) << std::setfill('0') << ( static_cast<unsigned int>(_Val) ) << ' ';
            return true;
        }
private:
        std::ostream &m_stream;
    };

    template < class _T >
    struct _SPrintableInserter
    {
        _SPrintableInserter( std::ostream &_stream ): m_stream(_stream)
        {
            m_stream << std::hex << std::uppercase;
        }
        bool operator()( typename _T::value_type _Val )
        {
            m_stream << (isprint(_Val) ? static_cast<char>(_Val) : '.' );
            return true;
        }
private:
        std::ostream &m_stream;
    };

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

        public:
            MiscCommon::ERRORCODE Start( bool _ClientMode = false );

        protected:
            void ThreadWorker( MiscCommon::INet::smart_socket *_SrvSocket, MiscCommon::INet::smart_socket *_CltSocket );

        private:
            MiscCommon::ERRORCODE _Start( bool _ClientMode );
            void SpawnServerMode();
            void SpawnClientMode();
            void ReportPackage( MiscCommon::INet::Socket_t _socket1, MiscCommon::INet::Socket_t _socket2, MiscCommon::BYTEVector_t &_buf )
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
                    ss << "\n\n";
                    // Printing string representation of the frame
                    _SPrintableInserter<MiscCommon::BYTEVector_t> pr_ins( ss );
                    for_each( _buf.begin(), _buf.end(), pr_ins );
                    //<< "\n" << std::string( reinterpret_cast<char*>( &_buf[ 0 ] ), _buf.size() ) << "\n"

                    ss << "\n\n";
                    // Printing hexadecimal representation of the frame
                    _SHexInserter<MiscCommon::BYTEVector_t> hex_ins( ss );
                    for_each( _buf.begin(), _buf.end(), hex_ins );
                    ss << "\n";
                }
                DebugLog( MiscCommon::erOK, ss.str() );
            }

        private:
            MiscCommon::INet::smart_socket m_ClientSocket;
            MiscCommon::INet::smart_socket m_ServerSocket;
            unsigned short m_nPort;
            MiscCommon::Thread_PTR_t m_thrd_clnt;
            MiscCommon::Thread_PTR_t m_thrd_srv;
            MiscCommon::Thread_PTR_t m_thrd_serversocket;
    };

}

#endif
