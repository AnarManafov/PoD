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
// v3
// added: cmdGET_WRK_NUM
//
namespace PROOFAgent
{
//=============================================================================
    enum ECmdType
    {
        // > > > > > protocol v2 < < < < <
        cmdUNKNOWN = -1,

        cmdVERSION = 10,
        //   cmdVERSION_BAD = cmdVERSION + 1, // NOT Implemented yet

        cmdHOST_INFO = 30,
        cmdGET_HOST_INFO = cmdHOST_INFO + 1,

        cmdUSE_PACKETFORWARDING_PROOF = 50,
        cmdUSE_DIRECT_PROOF = cmdUSE_PACKETFORWARDING_PROOF + 1,

        cmdSHUTDOWN = 60,
        // cmdRESTART = cmdSHUTDOWN + 1 // NOT Implemented yet
        // cmdIDLE_SHUTDOWN = cmdSHUTDOWN + 2, // NOT Implemented yet

        cmdGET_ID = 70,
        cmdID = cmdGET_ID + 1,
        cmdSET_ID = cmdGET_ID + 2,

        // cmdKillPROOFSERV =  // NOT Implemented yet

        // > > > > > protocol v3 < < < < <
        cmdGET_WRK_NUM = 80
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
        size_t size() const
        {
            return sizeof( m_version );
        }
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data ) const;
        bool operator== ( const SVersionCmd &val ) const
        {
            return ( m_version == val.m_version );
        }

        uint16_t m_version;
    };
    inline std::ostream &operator<< ( std::ostream &_stream, const SVersionCmd &val )
    {
        return _stream << val.m_version;
    }
//=============================================================================
    struct SHostInfoCmd: public SBasicCmd<SHostInfoCmd>
    {
        SHostInfoCmd(): m_proofPort( 0 )
        {
        }
        size_t size() const
        {
            size_t size( m_username.size() + 1 );
            size += m_host.size() + 1;
            size += sizeof( m_proofPort );
            return size;
        }
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data ) const;
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
    inline std::ostream &operator<< ( std::ostream &_stream, const SHostInfoCmd &val )
    {
        return _stream << val.m_username << ":" << val.m_host << ":" << val.m_proofPort;
    }
//=============================================================================
    struct SIdCmd: public SBasicCmd<SIdCmd>
    {
        SIdCmd(): m_id( 0 )
        {
        }
        void normalizeToLocal();
        void normalizeToRemote();
        size_t size() const
        {
            return sizeof( m_id );
        }
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data ) const;
        bool operator== ( const SIdCmd &_val ) const
        {
            return ( m_id == _val.m_id );
        }

        uint32_t m_id;
    };
    inline std::ostream &operator<< ( std::ostream &_stream, const SIdCmd &_val )
    {
        return _stream << _val.m_id;
    }
//=============================================================================

}

#endif /* PROTOCOLMESSAGES_H_ */
