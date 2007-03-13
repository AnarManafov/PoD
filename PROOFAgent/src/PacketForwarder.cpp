/************************************************************************/
/**
 * @file PacketForwarder.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                                    2007-03-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// PROOFAgent
#include "ErrorCode.h"
#include "PacketForwarder.h"

// BOOST
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;
using namespace std;

const unsigned int g_BUF_SIZE = 1024;

void CPacketForwarder::ThreadWorker( smart_socket *_SrvSocket, smart_socket *_CltSocket )
{
    BYTEVector_t buf( g_BUF_SIZE );
    // Macking non-blocking sockets
    _SrvSocket->set_nonblock();
    _CltSocket->set_nonblock();

    fd_set readset;
    while ( 1 )
    {
        FD_ZERO( &readset );
        FD_SET( *_SrvSocket, &readset );
        // Setting time-out
        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        if ( ::select( *_SrvSocket + 1, &readset, NULL, NULL, &timeout ) < 0 )
        {
            FaultLog( erError, "Error while calling \"select\"" );
            return ;
        }

        if ( FD_ISSET( *_SrvSocket, &readset ) )
        {
            try
            {
                *_SrvSocket >> &buf;
                WriteBuffer( buf, *_CltSocket );
                buf.clear();
                buf.resize( g_BUF_SIZE );
            }
            catch ( exception & e )
            {
                FaultLog( erError, e.what() );
                return ;
            }
        }
    }
}

ERRORCODE CPacketForwarder::Start()
{
    try
    {
        CSocketServer server;
        server.Bind( m_nPort );
        server.Listen( 1 ); // TODO: reuse parent socket socket  here
        m_ServerCocket = server.Accept();

        // executing PF threads
        boost::thread thrd_clnt( boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ServerCocket, &m_ClientSocket ) );
        boost::thread thrd_srv( boost::bind( &CPacketForwarder::ThreadWorker, this, &m_ClientSocket, &m_ServerCocket ) );
        thrd_clnt.join();
        thrd_srv.join();
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
        return erError;
    }
    return erOK;
}

void CPacketForwarder::WriteBuffer( BYTEVector_t &_Buf, smart_socket &_socket ) throw ( exception )
{
    //  boost::mutex::scoped_lock lock(m_Buf_mutex);
    string strSocketInfo;
    socket2string( _socket, &strSocketInfo );
    DebugLog( erOK, strSocketInfo + " BEGIN>>>" + string( reinterpret_cast<char*>( &_Buf[ 0 ] ) ) + "<<<END" );
    _socket << _Buf;
}
