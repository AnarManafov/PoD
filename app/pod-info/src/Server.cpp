//
//  Server.cpp
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "Server.h"
//=============================================================================
using namespace pod_info;
using namespace std;
//=============================================================================
CServer::CServer( const string &_host, int _port ):
    m_host( _host ),
    m_port( _port )
{

}
//=============================================================================