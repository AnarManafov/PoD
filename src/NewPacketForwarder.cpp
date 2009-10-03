/************************************************************************/
/**
 * @file NewPacketForwarder.cpp
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-09-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "NewPacketForwarder.h"
//=============================================================================
namespace PROOFAgent
{
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
        m_1stSockBasedContainer.insert( container_type::value_type( _node->first(), _node ) );
        m_2ndSockBasedContainer.insert( container_type::value_type( _node->second(), _node ) );
    }

//=============================================================================
    void CNodeContainer::removeNode1stBase( MiscCommon::INet::Socket_t _fd )
    {
        container_type::iterator found = m_1stSockBasedContainer.find( _fd );
        if ( m_1stSockBasedContainer.end() != found )
            m_1stSockBasedContainer.erase( found );
    }

//=============================================================================
    void CNodeContainer::removeNode2ndBase( MiscCommon::INet::Socket_t _fd )
    {
        container_type::iterator found = m_2ndSockBasedContainer.find( _fd );
        if ( m_2ndSockBasedContainer.end() != found )
        	m_2ndSockBasedContainer.erase( found );
    }

//=============================================================================
    SNode *CNodeContainer::getNode1stBase( MiscCommon::INet::Socket_t _fd )
    {
        container_type::const_iterator found = m_1stSockBasedContainer.find( _fd );
        if ( m_1stSockBasedContainer.end() != found )
            return found->second.get();

        return NULL;
    }

//=============================================================================
    SNode *CNodeContainer::getNode2ndBase( MiscCommon::INet::Socket_t _fd )
    {
        container_type::const_iterator found = m_2ndSockBasedContainer.find( _fd );
        if ( m_2ndSockBasedContainer.end() != found )
            return found->second.get();

        return NULL;
    }

}
