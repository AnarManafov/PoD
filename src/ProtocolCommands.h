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
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size()
        {
            return sizeof( m_version );
        }
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data );
        bool operator== ( const SVersionCmd &val ) const
        {
            return ( m_version == val.m_version );
        }

        uint16_t m_version;
    };
    inline std::ostream &operator<< (std::ostream &_stream, const SVersionCmd &val)
    {
    	return _stream << val.m_version;
    }
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
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data );
        bool operator== ( const SHostInfoCmd &val ) const
        {
            return ( m_username == val.m_username &&
                     m_host == val.m_host &&
                     m_proofPort == val.m_proofPort );
        }

        std::string m_username;
        std::string m_host;
        uint16_t m_proofPort;
    };
    inline std::ostream &operator<< (std::ostream &_stream, const SHostInfoCmd &val)
    {
    	return _stream << val.m_username << ":" << val.m_host << ":" << val.m_proofPort;
    }
//=============================================================================

}

#endif /* PROTOCOLMESSAGES_H_ */
