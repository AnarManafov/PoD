//
//  MessageParser.h
//  PoD
//
//  Created by Anar Manafov on 01.06.11.
//  Copyright 2011 GSI. All rights reserved.
//
#ifndef CMSG_PARSER_POD_REMOTE_H
#define CMSG_PARSER_POD_REMOTE_H
//=============================================================================
//STD
#include <string>
// API
#include <sys/select.h>
#include <errno.h>
// MiscCommon
#include "ErrorCode.h"
#include "def.h"
//=============================================================================
extern MiscCommon::LPCSTR g_message_OK;
//=============================================================================
namespace pod_remote
{
    template<class T, class T2>
    class CMessageParser
    {
        public:
            CMessageParser( int _fOut, int _fErr ):
                m_fOut( _fOut ),
                m_fErr( _fErr )
            {
            }

            void parse( T &_external_parser, T2 &_log )
            {
                const int read_size = 64;
                char buf[read_size];
                std::string err_out;
                std::string std_out;
                while( true )
                {
                    fd_set readset;
                    FD_ZERO( &readset );
                    FD_SET( m_fOut, &readset );
                    FD_SET( m_fErr, &readset );
                    int fd_max = m_fOut > m_fErr ? m_fOut : m_fErr;
                    int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );

                    if( EBADF == errno )
                        break;

                    if( retval < 0 )
                    {
                        std::stringstream ss;
                        ss << "Communication error: " << MiscCommon::errno2str();
                        _log( ss.str() );
                        break;
                    }

                    // receive error stream
                    if( FD_ISSET( m_fErr, &readset ) )
                    {
                        while( true )
                        {
                            int numread = read( m_fErr, buf, read_size );
                            if( 0 == numread )
                                return;

                            if( numread > 0 )
                            {
                                for( int i = 0; i < numread; ++i )
                                {
                                    err_out += buf[i];
                                    if( '\n' == buf[i] )
                                    {
                                        _log( "remote end reports: " + err_out );
                                        err_out.clear();
                                    }
                                }
                            }
                            else
                                break;
                        }
                    }
                    // receive stdout stream
                    if( FD_ISSET( m_fOut, &readset ) )
                    {
                        while( true )
                        {
                            int numread = read( m_fOut, buf, read_size );
                            if( 0 == numread )
                                return;

                            if( numread > 0 )
                            {
                                for( int i = 0; i < numread; ++i )
                                {
                                    std_out += buf[i];
                                    if( '\n' == buf[i] )
                                    {
                                        // call the external parser
                                        if( _external_parser( std_out ) )
                                            return;
                                    }
                                }
                            }
                            else
                                break;
                        }
                    }

                }
            }

        private:
            int m_fOut;
            int m_fErr;
    };
//=============================================================================
    struct SMessageParserOK
    {
        bool operator()( const std::string &_buf );
    };
//=============================================================================
    struct SMessageParserNumber
    {
            SMessageParserNumber(): m_num( 0 )
            {
            }

            bool operator()( const std::string &_buf );
            const int get() const;

        private:
            int m_num;
    };
//=============================================================================
    struct SMessageParserString
    {
            bool operator()( const std::string &_buf );
            const std::string get() const;

        private:
            std::string m_str;
    };

}
#endif
