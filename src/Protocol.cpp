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
#include "INet.h"
// pod-agent
#include "Protocol.h"
//=============================================================================
using namespace PROOFAgent;
using namespace MiscCommon;
using namespace MiscCommon::INet;
//=============================================================================
const size_t HEADER_SIZE = sizeof( SMessageHeader );
//=============================================================================
//=============================================================================
//=============================================================================
BYTEVector_t PROOFAgent::createMsg( uint16_t _cmd, const BYTEVector_t &_data )
{
    BYTEVector_t ret_val;
    return ret_val;

}
//=============================================================================
SMessageHeader PROOFAgent::parseMsg( BYTEVector_t *_data, const BYTEVector_t &_msg )
{
	return SMessageHeader();
}
//=============================================================================
//=============================================================================
//=============================================================================
CProtocol::CProtocol():
        m_ver( 2 ), // protocol version
        m_readAlready( 0 ),
        m_headerData( HEADER_SIZE )
{
}
//=============================================================================
CProtocol::~CProtocol()
{
}
//=============================================================================
void CProtocol::getDataAndRefresh( uint16_t &_cmd, MiscCommon::BYTEVector_t *_data )
{
    _cmd = m_msgHeader.m_cmd;

    m_headerData.clear();
    m_readAlready = 0;

    m_msgHeader.clear();

    if ( _data )
        _data->swap( m_curDATA );
    m_curDATA.clear();
}
//=============================================================================
CProtocol::EStatus_t CProtocol::read( int _socket )
{
    // always assume we use a non-blocking sockets

    // get header first
    if ( !m_msgHeader.isValid() )
    {
        while ( HEADER_SIZE != m_readAlready )
        {
            // need to read more to complete the header
            const ssize_t bytes_read = ::recv( _socket, &m_headerData[m_readAlready],
                                               HEADER_SIZE - m_readAlready, 0 );
            if ( 0 == bytes_read )
                return stDISCONNECT;

            if ( bytes_read < 0 )
            {
                if ( ECONNRESET == errno || ENOTCONN == errno )
                    return stDISCONNECT;

                if ( EAGAIN == errno || EWOULDBLOCK == errno )
                    return stAGAIN;

                throw system_error( "Error occurred while reading message's header." );
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
        const ssize_t bytes_read = ::recv( _socket, &m_curDATA[m_readAlready],
                                           m_msgHeader.m_len - m_readAlready, 0 );
        if ( 0 == bytes_read )
            return stDISCONNECT;

        if ( bytes_read < 0 )
        {
            if ( ECONNRESET == errno || ENOTCONN == errno )
                return stDISCONNECT;

            if ( EAGAIN == errno || EWOULDBLOCK == errno )
                return stAGAIN;

            throw system_error( "Error occurred while reading message's data" );
        }

        m_readAlready += bytes_read;
    }
    return stOK;
}
//=============================================================================
void CProtocol::write( int _socket, uint16_t _cmd, const MiscCommon::BYTEVector_t &_data )
{
    SMessageHeader header;
    strncpy( header.m_sign, "<POD_CMD>", sizeof( header.m_sign ) );
    header.m_cmd = _normalizeWrite16( _cmd );
    header.m_len = _normalizeWrite32( _data.size() );

    sendall( _socket, reinterpret_cast<unsigned char *>( &header ), sizeof( SMessageHeader ), 0 );
    sendall( _socket, &_data[0], _data.size(), 0 );
}
//=============================================================================
