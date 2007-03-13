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

struct SPFThread
{
    SPFThread( CPacketForwarder * _pThis, smart_socket *_SrvSocket, smart_socket *_CltSocket ) :
            m_pThis( _pThis ),
            m_SrvSocket( _SrvSocket ),
            m_CltSocket( _CltSocket ),
            m_Buf( g_BUF_SIZE )
    {}
    ~SPFThread()
    {
        m_SrvSocket->deattach();
        m_CltSocket->deattach();
    }
    void operator() ()
    {
        // m_SrvSocket.set_nonblock();
        fd_set readset;
        while ( 1 )
        {
            FD_ZERO( &readset );
            FD_SET( *m_SrvSocket, &readset );
            // Setting time-out
            timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            if ( ::select( *m_SrvSocket + 1, &readset, NULL, NULL, &timeout ) <= 0 )
                throw runtime_error( "Error while colling \"selkect\"" );

            if ( FD_ISSET( *m_SrvSocket, &readset ) )
            {
                try
                {
                    *m_SrvSocket >> &m_Buf;
                    m_pThis->WriteBuffer( m_Buf, *m_CltSocket );
                    m_Buf.clear();
                    m_Buf.resize( g_BUF_SIZE );
                }
                catch ( exception & e )
                {
                    m_pThis->LogThread( e.what() );
                }
            }
        }
    }
private:
    CPacketForwarder *m_pThis;
    smart_socket *m_SrvSocket;
    smart_socket *m_CltSocket;
    BYTEVector_t m_Buf;
};

void CPacketForwarder::Thread_Worker( smart_socket *_SrvSocket, smart_socket *_CltSocket )
{
    SPFThread s(this, _SrvSocket, _CltSocket);
    s();
}

ERRORCODE CPacketForwarder::Start()
{
    try
    {
        CSocketServer server;
        server.Bind( m_nPort );
        server.Listen( 1 ); // TODO: reuse parent socket socket  here
        smart_socket socket( server.Accept() );
        socket.set_nonblock();
        boost::thread thrd_clnt( boost::bind( &CPacketForwarder::Thread_Worker, this, &socket, &m_ClientSocket ) );
        boost::thread thrd_srv( boost::bind( &CPacketForwarder::Thread_Worker, this, &m_ClientSocket, &socket ) );
        //boost::thread thrd_clnt( SPFThread(this, socket, m_ClientSocket) );
        //   boost::thread thrd_srv( SPFThread( this, m_ClientSocket, socket ) );
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
    LogThread( "PF writes: " + string( reinterpret_cast<char*>( &_Buf[ 0 ] ) ) );
    _socket << _Buf;
}
