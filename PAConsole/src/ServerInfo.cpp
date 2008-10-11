/************************************************************************/
/**
 * @file ServerInfo.cpp
 * @brief Implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-05-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
// MiscCommon
#include "Process.h"
#include "def.h"
// PAConsole
#include "ServerInfo.h"

using namespace std;
using namespace MiscCommon;

pid_t CServerInfo::_IsRunning( const string &_Srv ) const
{
    return getprocbyname( _Srv );
}

bool CServerInfo::IsRunning( bool _check_all ) const
{
    const pid_t pidXrootD = IsXROOTDRunning();
    const pid_t pidPA = IsPROOFAgentRunning();
    if ( _check_all )
        return ( pidXrootD && pidPA );
    else
        return ( pidXrootD || pidPA );
}

pid_t CServerInfo::IsXROOTDRunning() const
{
    return _IsRunning( "xrootd" );
}

pid_t CServerInfo::IsPROOFAgentRunning() const
{
    return _IsRunning( "proofagent" );
}

string CServerInfo::GetXROOTDInfo() const
{
    const pid_t pid = IsXROOTDRunning();

    stringstream spid;
    if ( pid )
        spid << " <" << pid << ">";

    stringstream ss;
    ss
    << "XROOTD" << spid.str() << ": is "
    << ( pid ? "running" : "not running" );

    return ss.str();
}

string CServerInfo::GetPAInfo() const
{
    const pid_t pid = IsPROOFAgentRunning();

    stringstream spid;
    if ( pid )
        spid << " <" << pid << ">";

    stringstream ss;
    ss
    << "PROOFAgent" << spid.str() << ": is "
    << ( pid ? "running" : "not running" );

    string ver;
    GetPROOFAgentVersion( &ver );
    if ( !ver.empty() )
        ss << "\n" << "PROOFAgent's version:\n" << ver;

    return ss.str();
}

void CServerInfo::GetPROOFAgentVersion( std::string *_Ver ) const
{
    if ( !_Ver )
        return ;

    FILE *f = popen( "proofagent --version", "r" );
    if ( !f )
        return ;

    char * line = NULL;
    size_t len = 0;
    stringstream ss;
    while ( getline( &line, &len, f ) != -1 )
    {
        ss << line;
    }
    if ( line )
        free( line );
    pclose( f );
    ss << '\n';
    *_Ver = ss.str();
}
