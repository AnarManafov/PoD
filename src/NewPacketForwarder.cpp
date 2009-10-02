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
CNewPacketForwarder::CNewPacketForwarder() {
	// TODO Auto-generated constructor stub

}

//=============================================================================
CNewPacketForwarder::~CNewPacketForwarder() {
	// TODO Auto-generated destructor stub
}

//=============================================================================
void CNewPacketForwarder::addNode( const sock_type &_first, const sock_type &_second)
{
	SNode * node = new SNode(_first, _second);
	m_1stSockBasedContainer.insert( container_type::value_type(_first, node) );
	m_2ndSockBasedContainer.insert( container_type::value_type(_second, node) );
}
