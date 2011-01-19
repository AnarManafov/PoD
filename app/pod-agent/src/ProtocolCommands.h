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
#include <iterator>
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
// v4
// added: cmdUIConnect
// added: cmdUIConnectionReady
// added: cmdGET_WNs_LIST
// added: cmdWNs_LIST
namespace PROOFAgent
{
//=============================================================================
    enum ECmdType
    {
        cmdUNKNOWN = -1,

        cmdVERSION = 1,
        //   cmdVERSION_BAD = cmdVERSION + 1, // NOT Implemented yet

        // these command are used for PoD UI communication
        cmdUI_CONNECT = 5,
        cmdUI_CONNECT_READY = cmdUI_CONNECT + 1,

        cmdHOST_INFO = 10,
        cmdGET_HOST_INFO = cmdHOST_INFO + 1,

        cmdUSE_PACKETFORWARDING_PROOF = 20,
        cmdUSE_DIRECT_PROOF = cmdUSE_PACKETFORWARDING_PROOF + 1,

        cmdSHUTDOWN = 30,
        // cmdRESTART = cmdSHUTDOWN + 1 // NOT Implemented yet
        // cmdIDLE_SHUTDOWN = cmdSHUTDOWN + 2, // NOT Implemented yet

        cmdGET_ID = 40,
        cmdID = cmdGET_ID + 1,
        cmdSET_ID = cmdGET_ID + 2,

        // cmdKillPROOFSERV =  // NOT Implemented yet

        cmdGET_WRK_NUM = 50, // request a number of PROOF workers, which pod wn want to spawn
        cmdWRK_NUM = cmdGET_WRK_NUM + 1, // answer with number of PROOF wns,
        cmdGET_WNs_LIST = cmdGET_WRK_NUM + 2, // request a list of available workers
        cmdWNs_LIST = cmdGET_WRK_NUM + 3, // return a list of available workers
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
            size += m_version.size() + 1;
            size += m_PoDPath.size() + 1;
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
                     m_version == val.m_version &&
                     m_PoDPath == val.m_PoDPath &&
                     m_proofPort == val.m_proofPort );
        }

        std::string m_username;
        std::string m_host;
        std::string m_version;
        std::string m_PoDPath;
        uint16_t m_proofPort;
    };
    inline std::ostream &operator<< ( std::ostream &_stream, const SHostInfoCmd &val )
    {
        _stream
                << val.m_username << ":" << val.m_host << ":" << val.m_proofPort
                << ":" << val.m_version << ":" << val.m_PoDPath;
        return _stream;
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
    struct SWnListCmd: public SBasicCmd<SWnListCmd>
    {
        SWnListCmd()
        {
        }
        size_t size() const
        {
            size_t size( 0 );
            MiscCommon::StringVector_t::const_iterator iter = m_container.begin();
            MiscCommon::StringVector_t::const_iterator iter_end = m_container.end();
            for( ; iter != iter_end; ++iter )
            {
                size += iter->size() + 1;
            }

            return size;
        }
        void normalizeToLocal();
        void normalizeToRemote();
        void _convertFromData( const MiscCommon::BYTEVector_t &_data );
        void _convertToData( MiscCommon::BYTEVector_t *_data ) const;
        bool operator== ( const SWnListCmd &val ) const
        {
            return ( std::equal( m_container.begin(), m_container.end(),
                                 val.m_container.begin() ) );
        }

        MiscCommon::StringVector_t m_container;
    };
    inline std::ostream &operator<< ( std::ostream &_stream, const SWnListCmd &val )
    {
        std::ostream_iterator< std::string > output( _stream, "\n" );
        std::copy( val.m_container.begin(), val.m_container.end(), output );
        return _stream;
    }
}

#endif /* PROTOCOLMESSAGES_H_ */
