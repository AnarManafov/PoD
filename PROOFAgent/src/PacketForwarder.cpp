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

using namespace MiscCommon;
using namespace MiscCommon::INet;
using namespace PROOFAgent;

struct SPFThread
{
    SPFThread( CPacketForwarder * _pThis ) : m_pThis( _pThis )
    {}
    void operator() ()
    {
   /*     try
        {
            CSocketServer server;
            server.Bind( m_pThis->m_nPort );
            server.Listen( 1 );
            while ( true )
            {
                smart_socket ServerSocket( server.Accept() );

                // proxy beetwin: PROOF server -> ServerSocket and ClientSocket <- PROOF worker
                
            }
        }
        catch ( exception & e )
        {
            m_pThis->LogThread( e.what() );
        }*/
    }
    private:
        CPacketForwarder *m_pThis;
        smart_socket m_ClientSocket;
};

ERRORCODE CPacketForwarder::Start( )
{
    
}
