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
// MiscCommon
#include "ErrorCode.h"
#include "HexView.h"
// Agent
#include "Node.h"

using namespace std;
using namespace MiscCommon;

namespace PROOFAgent
{

//=============================================================================
// CNode
//=============================================================================
    int CNode::dealWithData( MiscCommon::INet::Socket_t _fd )
    {
        if ( !isValid() )
            return -1;

        sock_type *input = socketByFD( _fd );
        sock_type *output = pairedWith( _fd );

        m_bytesToSend = read_from_socket( *input, &m_buf );

        // DISCONNECT has been detected
        if ( m_bytesToSend <= 0 || !isValid() )
            return -1;

        sendall( *output, &m_buf[0], m_bytesToSend, 0 );

        // TODO: uncomment when log level is implemented
        //  BYTEVector_t tmp_buf( m_buf.begin(), m_buf.begin() + m_bytesToSend );
        //   ReportPackage( *input, *output, tmp_buf );
//    m_idleWatch.touch();

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



//=============================================================================
// CNodeContainer
//=============================================================================
    CNodeContainer::CNodeContainer()
    {
        // TODO Auto-generated constructor stub

    }

//=============================================================================
    CNodeContainer::~CNodeContainer()
    {
        // TODO Auto-generated destructor stub
    }

//=============================================================================
    void CNodeContainer::addNode( node_type _node )
    {
        // set sockets to O_NONBLOCK mode
        _node->setNonblock();

        m_sockBasedContainer.insert( container_type::value_type( _node->first(), _node ) );
        m_sockBasedContainer.insert( container_type::value_type( _node->second(), _node ) );
        m_nodes.insert( _node );
    }

//=============================================================================
    void CNodeContainer::removeNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            m_sockBasedContainer.erase( found );
    }

//=============================================================================
    void CNodeContainer::removeBadNodes()
    {
        // remove bad nodes
        container_type::iterator iter = m_sockBasedContainer.begin();
        container_type::iterator iter_end = m_sockBasedContainer.end();
        for ( ; iter != iter_end; )
        {
            if ( !iter->second->isActive() &&
                 !iter->second->isValid() &&
                 !iter->second->isInUse() )
            {
                m_nodes.erase( iter->second );
                m_sockBasedContainer.erase( iter++ );
            }
            else
            {
                ++iter;
            }
        }
    }

//=============================================================================
    CNodeContainer::node_type CNodeContainer::getNode( MiscCommon::INet::Socket_t _fd )
    {
        container_type::const_iterator found = m_sockBasedContainer.find( _fd );
        if ( m_sockBasedContainer.end() != found )
            return found->second;

        return node_type();
    }

}
