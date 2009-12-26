/************************************************************************/
/**
 * @file ProtocolCommands.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-22
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROTOCOLCOMMANDS_H_
#define PROTOCOLCOMMANDS_H_
//=============================================================================
// STD
#include <stdint.h>
#include <stdexcept>
// MiscCommon
#include "def.h"
// pod-agetn
#include "Protocol.h"
//=============================================================================
const uint16_t g_protocolVersion = 2;
//=============================================================================
// Protocol versions
// v2
// - init version
//
namespace PROOFAgent
{
//=============================================================================
    enum ECmdType
    {
        // > > > > > protocol v2 < < < < <
        cmdUNKNOWN = -1,

        cmdVERSION = 0,
        cmdGET_VERSION = 1,

        cmdHOST_INFO = 2,
        cmdGET_HOST_INFO = 3,

        cmdUSE_PROXYPROOF = 4,
        cmdUSE_DIRECTPROOF = 5
    };
//=============================================================================
    template<class _Owner>
    struct SBasicCmd
    {
        void convertFromData( const MiscCommon::BYTEVector_t &_data )
        {
            _Owner *p = reinterpret_cast<_Owner*>( this );
            p->_convertFromData( _data );
            p->normalizeToLocal();
        }
        void convertToData( MiscCommon::BYTEVector_t *_data )
        {
            _Owner *p = reinterpret_cast<_Owner*>( this );
            p->normalizeToRemote();
            p->_convertToData( _data );
            p->normalizeToLocal();
        }
    };
//=============================================================================
    struct SVersionCmd: public SBasicCmd<SVersionCmd>
    {
        SVersionCmd(): m_version( g_protocolVersion )
        {
        }
        void normalizeToLocal()
        {
            m_version = CProtocol::_normalizeRead16( m_version );
        }
        void normalizeToRemote()
        {
            m_version = CProtocol::_normalizeWrite16( m_version );
        }
        size_t size()
        {
            return sizeof( m_version );
        }
        void _convertFromData( const MiscCommon::BYTEVector_t &_data )
        {
            if ( _data.size() < size() )
                throw std::runtime_error( "Protocol message data is too short" );

            memcpy( &m_version, &_data[0], sizeof( m_version ) );
        }
        void _convertToData( MiscCommon::BYTEVector_t *_data )
        {
            _data->resize( size() );
            memcpy( &( *_data )[0], &m_version, _data->size() );
        }
        bool operator== ( const SVersionCmd &val )
        {
            return ( m_version == val.m_version );
        }
        uint16_t m_version;
    };
//=============================================================================
    struct SHostInfoCmd: public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd(): m_proofPort( 0 )
        {
        }
        size_t size()
        {
            size_t size( m_username.size() + 1 );
            size += m_host.size() + 1;
            size += sizeof( m_proofPort );
            return size;
        }
        void normalizeToLocal()
        {
            m_proofPort = CProtocol::_normalizeRead16( m_proofPort );
        }
        void normalizeToRemote()
        {
            m_proofPort = CProtocol::_normalizeWrite16( m_proofPort );
        }
        void _convertFromData( const MiscCommon::BYTEVector_t &_data )
        {
            size_t idx( 0 );
            MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
            MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
            for ( ; iter != iter_end; ++iter, ++idx )
            {
                char c( *iter );
                if ( '\0' == c )
                {
                    ++iter;
                    ++idx;
                    break;
                }
                m_username.push_back( c );
            }

            for ( ; iter != iter_end; ++iter, ++idx )
            {
                char c( *iter );
                if ( '\0' == c )
                {
                    ++iter;
                    ++idx;
                    break;
                }
                m_host.push_back( c );
            }

            if ( _data.size() < size() )
                throw std::runtime_error( "Protocol message data is too short" );

            m_proofPort = _data[idx++];
            m_proofPort += ( _data[idx] << 8 );
        }
        void _convertToData( MiscCommon::BYTEVector_t *_data )
        {
            std::copy( m_username.begin(), m_username.end(), std::back_inserter( *_data ) );
            _data->push_back( '\0' );
            std::copy( m_host.begin(), m_host.end(), std::back_inserter( *_data ) );
            _data->push_back( '\0' );


            _data->push_back( m_proofPort & 0xFF );
            _data->push_back( m_proofPort >> 8 );
        }
        bool operator== ( const SHostInfoCmd &val )
        {
            return ( m_username == val.m_username &&
                     m_host == val.m_host &&
                     m_proofPort == val.m_proofPort );
        }

//        uint32 GetInt32( uint8 *pBytes )
//        {
//        return (uint32)(*(pBytes + 3) << 24 | *(pBytes + 2) << 16 | *(pBytes + 1) << 8 | *pBytes);
//        }
//
//        void Int32ToUInt8Arr( int32 val, uint8 *pBytes )
//        {
//        pBytes[0] = (uint8)val;
//        pBytes[1] = (uint8)(val >> 8);
//        pBytes[2] = (uint8)(val >> 16);
//        pBytes[3] = (uint8)(val >> 24);
//        }

//        long x=0x67452301;
//        unsigned char y[4];
//
//        y[0] = x & 0x000000ff;        //y[0] is the least significant byte
//        y[1] = (x & 0x0000ff00) >> 8;
//        y[2] = (x & 0x00ff0000) >> 16;
//        y[3] = (x & 0xff000000) >> 24
        std::string m_username;
        std::string m_host;
        uint16_t m_proofPort;
    };
//=============================================================================

}

#endif /* PROTOCOLMESSAGES_H_ */
