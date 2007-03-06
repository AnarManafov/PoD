/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// PROOFAgent
#include "ErrorCode.h"

#include "PacketForwarder.h"

using namespace MiscCommon;
using namespace PROOFAgent;


ERRORCODE CPacketForwarder::Start( )
{
   /* InfoLog( erOK, "Starting..." );
    smart_socket SocketListener( AF_INET, SOCK_STREAM, 0 );
    if ( SocketListener < 0 )
    {
        FaultLog( erOK, "Soket error..." ); // TODO: perror( "socket" );
        return ;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ::htons( m_nPort );
    addr.sin_addr.s_addr = ::htonl( INADDR_ANY );
    if ( bind( SocketListener, ( struct sockaddr * ) & addr, sizeof( addr ) ) < 0 )
    {
        FaultLog( erOK, "Soket bind error..." ); // TODO: perror( "bind" );
        return ;
    }

    ::listen( SocketListener, 1 );
    std::stringstream ssMsg;
    ssMsg << "Listenening on port #" << m_pThis->m_Data.m_nPort << " ...";
    m_pThis->LogThread( ssMsg.str() );
    while ( true )
    {
        smart_socket sock( ::accept( SocketListener, NULL, NULL ) );
        if ( sock < 0 )
        {
            FaultLog( erOK, "Soket accept error..." ); // TODO: perror("accept");
            return ;
        }
        while ( 1 )
        {
            bytes_read = ::recv( sock, buf, 1024, 0 );
            if ( bytes_read <= 0 ) break;
            ::send( sock, buf, bytes_read, 0 );
        }*/
    }
