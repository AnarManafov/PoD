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
#ifndef PROOFAGENTPACKETFORWARDER_H
#define PROOFAGENTPACKETFORWARDER_H

// BOOST
#include <boost/thread/mutex.hpp>

// STD
//#include <algorithm>

// Our
#include "INet.h"
#include "LogImp.h"
#include "def.h"

namespace PROOFAgent
{

    class CPacketForwarder: MiscCommon::CLogImp<CPacketForwarder>
    {
        public:
            CPacketForwarder( MiscCommon::INet::Socket_t &_Socket, unsigned short _nPort ) :
                    m_ClientSocket( _Socket ),
                    m_nPort( _nPort )
            {}
            ~CPacketForwarder()
            {}
            REGISTER_LOG_MODULE( PacketForwarder )
                            
            void LogThread( const std::string _Msg, MiscCommon::ERRORCODE _erCode = MiscCommon::erOK )
            {
                InfoLog( _erCode, _Msg );
            }

            MiscCommon::ERRORCODE ReadBuffer( MiscCommon::BYTEVector_t *_Buf )
            {
                if ( !_Buf )
                    MiscCommon::erNULLArg;
                boost::mutex::scoped_lock lock(m_Buf_mutex);
                _Buf->clear();
                std::copy( m_Buf.begin(), m_Buf.end(), std::back_inserter(*_Buf) );
            }
            
            MiscCommon::ERRORCODE WriteBuffer( MiscCommon::BYTEVector_t *_Buf )
            {
                if ( !_Buf )
                    MiscCommon::erNULLArg;
                boost::mutex::scoped_lock lock(m_Buf_mutex);
                m_Buf.resize(_Buf->size());
                std::swap(*_Buf, m_Buf);
            }

        public:
            MiscCommon::ERRORCODE Start( );

        private:
            MiscCommon::INet::smart_socket m_ClientSocket;
            unsigned short m_nPort;
            MiscCommon::BYTEVector_t m_Buf;
            boost::mutex m_Buf_mutex;
    };

}

#endif
