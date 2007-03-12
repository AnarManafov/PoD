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

using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;
using namespace std;

const unsigned int g_BUF_SIZE = 1024;

struct SPFThread
{
    SPFThread( CPacketForwarder * _pThis, smart_socket &_SrvSocket, smart_socket &_CltSocket ) :
            m_pThis( _pThis ),
            m_SrvSocket( _SrvSocket ),
            m_CltSocket( _CltSocket ),
            m_Buf( g_BUF_SIZE )
    {}
    ~SPFThread()
    {
        m_SrvSocket.deattach();
        m_CltSocket.deattach();
    }
    void operator() ()
    {
        try
        {
            while ( 1 )
            {
                m_SrvSocket >> &m_Buf;
                m_pThis->WriteBuffer( m_Buf, m_CltSocket );
                m_Buf.clear();
                m_Buf.resize( g_BUF_SIZE );
            }
        }
        catch ( exception & e )
        {
            m_pThis->LogThread( e.what() );
        }
    }
private:
    CPacketForwarder *m_pThis;
    smart_socket &m_SrvSocket;
    smart_socket &m_CltSocket;
    BYTEVector_t m_Buf;
};

ERRORCODE CPacketForwarder::Start( bool _JoinThreads )
{
    try
    {
        CSocketServer server;
        server.Bind( m_nPort );
        server.Listen( 1 ); // TODO: reuse parent socket socket  here
        smart_socket socket( server.Accept() );
        boost::thread thrd_clnt( SPFThread( this, socket, m_ClientSocket ) );
        boost::thread thrd_srv( SPFThread( this, m_ClientSocket, socket ) );
        if ( _JoinThreads )
        {
            thrd_clnt.join();
            thrd_srv.join();
        }
    }
    catch ( exception & e )
    {
        FaultLog( erError, e.what() );
        return erError;
    }
}

void CPacketForwarder::WriteBuffer( BYTEVector_t &_Buf, smart_socket &_socket ) throw ( exception )
{
    //  boost::mutex::scoped_lock lock(m_Buf_mutex);
    _socket << _Buf;
}
