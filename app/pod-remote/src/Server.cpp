//
//  Server.cpp
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "Server.h"
// MiscCommon
#include "def.h"
//=============================================================================
using namespace pod_info;
using namespace std;
using namespace PROOFAgent;
using namespace MiscCommon;
//=============================================================================
CServer::CServer( const string &_host, unsigned int _port ):
    m_host( _host ),
    m_port( _port )
{
}
//=============================================================================
void CServer::getSrvHostInfo( PROOFAgent::SHostInfoCmd *_srvHostInfo ) const
{
    if( 0 == m_port || m_host.empty() )
    {
        throw runtime_error( "Wrong server url." );
    }
    inet::CSocketClient m_socket;
    m_socket.connect( m_port, m_host );
    BYTEVector_t data;
    processAdminConnection( &data, m_socket.getSocket(), Req_Host_Info );

    _srvHostInfo->convertFromData( data );
}
//=============================================================================
void CServer::getListOfWNs( PROOFAgent::SWnListCmd *_lst ) const
{
    if( 0 == m_port || m_host.empty() )
    {
        throw runtime_error( "Wrong server url." );
    }
    inet::CSocketClient m_socket;
    m_socket.connect( m_port, m_host );
    BYTEVector_t data;
    processAdminConnection( &data, m_socket.getSocket(), Req_WNs_List );

    _lst->convertFromData( data );
}
//=============================================================================
void CServer::processAdminConnection( BYTEVector_t *_data,
                                      int _serverSock, CServer::Requests _req ) const
{
    CProtocol protocol;
    SVersionCmd v;
    BYTEVector_t ver;
    v.convertToData( &ver );
    protocol.write( _serverSock, static_cast<uint16_t>( cmdUI_CONNECT ), ver );
    bool bProcessNoMore( false );
    while( !bProcessNoMore )
    {
        CProtocol::EStatus_t ret = protocol.read( _serverSock );
        switch( ret )
        {
            case CProtocol::stDISCONNECT:
                throw runtime_error( "a disconnect has been detected on the adminChannel" );
            case CProtocol::stAGAIN:
            case CProtocol::stOK:
                {
                    while( protocol.checkoutNextMsg() )
                    {
                        bProcessNoMore = ( processProtocolMsgs( _data, _serverSock, &protocol, _req ) > 0 );
                        if( bProcessNoMore )
                            break;
                    }
                }
                break;
        }
    }
}
//=============================================================================
int CServer::processProtocolMsgs( BYTEVector_t *_data, int _serverSock,
                                  CProtocol * _protocol, CServer::Requests _req ) const
{
    BYTEVector_t data;
    SMessageHeader header = _protocol->getMsg( &data );
    switch( static_cast<ECmdType>( header.m_cmd ) )
    {
        case cmdUI_CONNECT_READY:
            {
                switch( _req )
                {
                    case Req_Host_Info:
                        _protocol->writeSimpleCmd( _serverSock, static_cast<uint16_t>( cmdGET_HOST_INFO ) );
                        break;
                    case Req_WNs_List:
                        _protocol->writeSimpleCmd( _serverSock, static_cast<uint16_t>( cmdGET_WNs_LIST ) );
                        break;
                }
            }
            break;
        case cmdHOST_INFO:
        case cmdWNs_LIST:
            _data->swap( data );
            return 1;
        case cmdSHUTDOWN:
            throw runtime_error( "Server requests to shut down. Stop admin channel..." );
        default:
            throw runtime_error( "Unexpected message in the admin channel" );
    }
    return 0;
}
