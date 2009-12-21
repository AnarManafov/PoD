/************************************************************************/
/**
 * @file Protocol.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-07
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// API
#include <sys/socket.h>
// MiscCommon
#include "ErrorCode.h"
// pod-agent
#include "Protocol.h"
//=============================================================================
using namespace PROOFAgent;
using namespace MiscCommon;
//=============================================================================
const size_t HEADER_SIZE = sizeof( SMessageHeader );
//=============================================================================
CProtocol::CProtocol():
        m_ver( 2 ), // protocol version
        m_readAlready( 0 ),
        m_headerData( HEADER_SIZE ),
        m_curCMD( NULL_VAL )
{
}
//=============================================================================
CProtocol::~CProtocol()
{
}
//=============================================================================
void CProtocol::getDataAndRefresh( MiscCommon::BYTEVector_t *_buf )
{
    m_headerData.clear();
    m_readAlready = 0;

    m_msgHeader.clear();

    if ( _buf )
        _buf->swap( m_curDATA );
    m_curDATA.clear();
}
//=============================================================================
CProtocol::EProtocolCMD_t CProtocol::read( int _socket )
{
    // get header
    if ( !m_msgHeader.isValid() )
    {
        while ( HEADER_SIZE != m_readAlready )
        {
            // need to read more to complete the header
            const ssize_t bytes_read = ::recv( _socket, &m_headerData[m_readAlready], HEADER_SIZE - m_readAlready, 0 );
            if ( 0 == bytes_read )
                return DISCONNECT;

            if ( bytes_read < 0 )
            {
                if ( ECONNRESET == errno || ENOTCONN == errno )
                    return DISCONNECT;

                if ( EAGAIN == errno || EWOULDBLOCK == errno )
                    return AGAIN;

                throw system_error( "" );
            }

            m_readAlready += bytes_read;
        }

        // HEADER has been read
        // check header
        memcpy( &m_msgHeader, &m_headerData[0], HEADER_SIZE );

        m_headerData.clear();
        m_readAlready = 0;

        if ( !m_msgHeader.isValid() )
            system_error( "bad message header" );

        m_msgHeader.m_cmd = _normalizeRead16( m_msgHeader.m_cmd );
        m_msgHeader.m_len = _normalizeRead32( m_msgHeader.m_len );
        m_curDATA.clear();
        m_curDATA.resize( m_msgHeader.m_len );
    }

    // get data block
    while ( m_msgHeader.m_len != m_readAlready )
    {
        // need to read more to complete the header
        const ssize_t bytes_read = ::recv( _socket, &m_curDATA[m_readAlready], m_msgHeader.m_len - m_readAlready, 0 );
        if ( 0 == bytes_read )
            return DISCONNECT;

        if ( bytes_read < 0 )
        {
            if ( ECONNRESET == errno || ENOTCONN == errno )
                return DISCONNECT;

            if ( EAGAIN == errno || EWOULDBLOCK == errno )
                return AGAIN;

            throw system_error( "" );
        }

        m_readAlready += bytes_read;
    }

    switch ( m_msgHeader.m_cmd )
    {
        case 4:
            return HOST_INFO;
        default:
            return UNKNOWN;
    }
}
