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
namespace PROOFAgent {
//=============================================================================
CNodeContainer::CNodeContainer() {
	// TODO Auto-generated constructor stub

}

//=============================================================================
CNodeContainer::~CNodeContainer() {
	// TODO Auto-generated destructor stub
}

//=============================================================================
void CNodeContainer::addNode( node_type _node )
{
	m_1stSockBasedContainer.insert( container_type::value_type(_node->m_first->get(), _node) );
	m_2ndSockBasedContainer.insert( container_type::value_type(_node->m_second->get(), _node) );
}

//=============================================================================
SNode *getNode1stBase( MiscCommon::INet::Socket_t _fd )
{
	return NULL;
}

//=============================================================================
SNode *getNode2ndBase( MiscCommon::INet::Socket_t _fd )
{
	return NULL;
}

}
