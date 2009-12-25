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
const size_t g_maxUsernameLength = 32;
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
            memcpy( p, &_data[0], sizeof( _Owner ) );
            p->normalizeToLocal();
        }
        void convertToData( const MiscCommon::BYTEVector_t *_data )
        {
            _Owner *p = reinterpret_cast<_Owner*>( this );
            p->normalizeToRemote();
            memcpy( &_data[0], &p, sizeof( _Owner ) );
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

        uint16_t m_version;
    };
//=============================================================================
    struct SHostInfoCmd: public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd(): m_proofPort( 0 )
        {
        }
        void normalizeToLocal()
        {
            m_proofPort = CProtocol::_normalizeRead16( m_proofPort );
        }
        void normalizeToRemote()
        {
            m_proofPort = CProtocol::_normalizeWrite16( m_proofPort );
        }

        char m_username[g_maxUsernameLength];
        char m_host[_POSIX_HOST_NAME_MAX];
        uint16_t m_proofPort;
    };
//=============================================================================

}

#endif /* PROTOCOLMESSAGES_H_ */
