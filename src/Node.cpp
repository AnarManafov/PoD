/************************************************************************/
/**
 * @file Node.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// Agent
#include "Node.h"
// MiscCommon
#include "ErrorCode.h"
#include "HexView.h"

using namespace std;
using namespace MiscCommon;

namespace PROOFAgent
{

//=============================================================================
// CNode
//=============================================================================
    int CNode::dealWithData( ENodeSocket_t _which )
    {
        if ( !isValid() )
        {
            setInUse( false, _which );
            return -1;
        }

        sock_type *input( NULL );
        sock_type *output( NULL );
        BYTEVector_t *buf( NULL );

        switch ( _which )
        {
            case nodeSocketFirst:
                input = m_first;
                output = m_second;
                buf = &m_bufFirst;
                break;
            case nodeSocketSecond:
                input = m_second;
                output = m_first;
                buf = &m_bufSecond;
                break;
        }

        size_t m_bytesToSend = read_from_socket( *input, &( *buf ) );

        // DISCONNECT has been detected
        if ( m_bytesToSend <= 0 || !isValid() )
        {
            setInUse( false, _which );
            return -1;
        }

        sendall( *output, &( *buf )[0], m_bytesToSend, 0 );

        // TODO: uncomment when log level is implemented
        //  BYTEVector_t tmp_buf( m_buf.begin(), m_buf.begin() + m_bytesToSend );
        //   ReportPackage( *input, *output, tmp_buf );

        setInUse( false, _which );

        return 0;
    }

//=============================================================================
    void CNode::ReportPackage( MiscCommon::INet::Socket_t _socket1, MiscCommon::INet::Socket_t _socket2,
                               const BYTEVector_t &_buf )
    {
        string strSocket1;
        MiscCommon::INet::socket2string( _socket1, &strSocket1 );
        string strSocket2;
        MiscCommon::INet::socket2string( _socket2, &strSocket2 );

        stringstream ss;
        ss
        << strSocket1 << " > " << strSocket2
        << " (" << _buf.size() << " bytes): ";
        if ( !_buf.empty() )
        {
            ss
            << "\n"
            << BYTEVectorHexView_t( _buf )
            << "\n";
        }
        DebugLog( erOK, ss.str() );
    }

}
