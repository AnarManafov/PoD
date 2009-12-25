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
// MiscCommon
#include "def.h"
// pod-agetn
#include "Protocol.h"
//=============================================================================
const uint16_t g_protocolVersion = 2;
//const size_t g_maxUsernameLen = 32;
// _POSIX_HOST_NAME_MAX
//const size_t g_maxHostNameLen = 255;
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
            _data->resize( p->size() );

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
            memcpy( &m_version, &_data[0], _data.size() );
        }
        void _convertToData( MiscCommon::BYTEVector_t *_data )
        {
            memcpy( &( *_data )[0], &m_version, _data->size() );
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
            size_t size( m_username.size() );
            size += m_host.size();
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
            MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
            MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
            do
            {
                m_username += char( *iter );
            }
            while ( *iter != '\0' );

            do
            {
                m_host += char( *iter );
            }
            while ( *iter != '\0' );

            memcpy( &m_proofPort, &( *iter ), _data.size() );
        }
        void _convertToData( MiscCommon::BYTEVector_t *_data )
        {
            size_t idx( 0 );
            memcpy( &_data[idx], m_username.c_str(), m_username.size() + 1 );
            idx += m_username.size() + 1;
            memcpy( &_data[idx], m_host.c_str(), m_host.size() + 1 );
            idx += m_host.size() + 1;
            memcpy( &_data[idx], &m_proofPort, sizeof( m_proofPort ) );
        }
        std::string m_username;
        std::string m_host;
        uint16_t m_proofPort;
    };
//=============================================================================

}

#endif /* PROTOCOLMESSAGES_H_ */
