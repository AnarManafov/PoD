//
//  Environment.cpp
//  PoD
//
//  Created by Anar Manafov on 17.01.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "Environment.h"
// STD
#include <sstream>
#include <stdexcept>
#include <fstream>
// MiscCommon
#include "MiscUtils.h"
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
CEnvironment::CEnvironment(): m_isLocalServer( false )
{

}
//=============================================================================
void CEnvironment::init()
{
    char *pod_location;
    pod_location = getenv( "POD_LOCATION" );
    if( NULL == pod_location )
        throw runtime_error( "POD_LOCATION is not defined. Please, initialize PoD environment." );

    m_PoDPath = pod_location;
    smart_path( &m_PoDPath );
    smart_append( &m_PoDPath, '/' );

    getLocalVersion();
    checkForLocalServer();
}
//=============================================================================
void CEnvironment::getLocalVersion()
{
    string version_file_name( m_PoDPath );
    version_file_name += "etc/version";
    ifstream f( version_file_name.c_str() );
    if( !f.is_open() )
        throw runtime_error( "Can't open PoD version file."
                             " Probably PoD installation is broken."
                             " You may want to reinstall PoD to repair the installation." );
    f >> m_localVer;
}
//=============================================================================
bool CEnvironment::checkForLocalServer()
{
    string userPoD( "~/.PoD/" );
    smart_path( &userPoD );
    userPoD += "etc/server_info.cfg";
    m_isLocalServer = does_file_exists( userPoD );
    
    return m_isLocalServer;
}
