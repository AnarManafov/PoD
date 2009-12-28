/************************************************************************/
/**
 * @file Protocol.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-07
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROTOCOL_H_
#define PROTOCOL_H_
//=============================================================================
// STD
#include <cstring>
// API
#include <arpa/inet.h>
// MiscCommon
#include "def.h"
//=============================================================================
namespace PROOFAgent
{
//=============================================================================
// protocol ver2
// a very simple protocol
// | <POD_CMD> (10) char | CMD (2) uint16_t | LEN (4) uint32_t | DATA (LEN) unsigned char |
    struct SMessageHeader
    {
        SMessageHeader():
                m_cmd( 0 ),
                m_len( 0 )
        {
        }
        char m_sign[10];
        uint16_t m_cmd;
        uint32_t m_len;

        bool isValid()
        {
            return( strcmp( m_sign, "<POD_CMD>" ) == 0 );
        }
        void clear()
        {
            m_sign[0] = '\0';
            m_cmd = 0;
            m_len = 0;
        }
    };
//=============================================================================
    MiscCommon::BYTEVector_t createMsg( uint16_t _cmd, const MiscCommon::BYTEVector_t &_data );
//=============================================================================
    SMessageHeader parseMsg( MiscCommon::BYTEVector_t *_data, const MiscCommon::BYTEVector_t &_msg );
//=============================================================================
    class CProtocol
    {
        public:
            CProtocol();
            virtual ~CProtocol();

            typedef enum EStatus
            {
                stDISCONNECT = -3,
                stAGAIN = -2,
                stUNKNOWN = -1,
                stOK = 0
            } EStatus_t;

            EStatus_t read( int _socket );
            void write( int _socket, uint16_t _cmd, const MiscCommon::BYTEVector_t &_data );
            void getDataAndRefresh( uint16_t &_cmd, MiscCommon::BYTEVector_t *_data );

        public:
            // The following 4 functions convert values between host and network byte order.
            // Whenever data should be send to a remote peer the _normalizeWrite must be used.
            // Whenever data are going to be read from the _normalizeRead must be used to check that Endianness is correct.
            static uint16_t _normalizeRead16( uint16_t _value )
            {
                return ntohs( _value );
            }
            static uint32_t _normalizeRead32( uint32_t _value )
            {
                return ntohl( _value );
            }
            static uint16_t _normalizeWrite16( uint16_t _value )
            {
                return htons( _value );
            }
            static uint32_t _normalizeWrite32( uint32_t _value )
            {
                return htonl( _value );
            }

        private:
            uint16_t m_ver;
            MiscCommon::BYTEVector_t m_buffer;

            SMessageHeader m_msgHeader;
            MiscCommon::BYTEVector_t m_curDATA;
    };

}

#endif /* PROTOCOL_H_ */
