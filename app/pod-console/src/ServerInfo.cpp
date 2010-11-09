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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// MiscCommon
#include "Process.h"
#include "def.h"
// PAConsole
#include "ServerInfo.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
// TODO: we need check processes and its user name, not only existence of the process
pid_t CServerInfo::_IsRunning( const string &_Srv ) const
{
    vectorPid_t pids = getprocbyname( _Srv, true );
    if ( pids.empty() )
        return 0;

    // TODO: send an error if there are more than one pid
    return pids[0];
}
//=============================================================================
bool CServerInfo::IsRunning( bool _check_all ) const
{
    const pid_t pidXProofD = IsXPROOFDRunning();
    const pid_t pidPA = IsPROOFAgentRunning();
    if ( _check_all )
        return ( pidXProofD && pidPA );
    else
        return ( pidXProofD || pidPA );
}
//=============================================================================
pid_t CServerInfo::IsXPROOFDRunning() const
{
    return _IsRunning( "xproofd" );
}
//=============================================================================
pid_t CServerInfo::IsPROOFAgentRunning() const
{
    return _IsRunning( "pod-agent" );
}
//=============================================================================
string CServerInfo::GetXPROOFDInfo() const
{
    const pid_t pid = IsXPROOFDRunning();

    stringstream spid;
    if ( pid )
        spid << " <" << pid << ">";

    stringstream ss;
    ss
    << "xproofd" << spid.str() << ": is "
    << ( pid ? "running" : "NOT RUNNING" );

    return ss.str();
}
//=============================================================================
string CServerInfo::GetPAInfo() const
{
    const pid_t pid = IsPROOFAgentRunning();

    stringstream spid;
    if ( pid )
        spid << " <" << pid << ">";

    stringstream ss;
    ss
    << "pod-agent" << spid.str() << ": is "
    << ( pid ? "running" : "NOT RUNNING" );

    string ver;
    GetPROOFAgentVersion( &ver );
    if ( !ver.empty() )
        ss << "\n" << "pod-agent version:\n" << ver;

    return ss.str();
}
//=============================================================================
void CServerInfo::GetPROOFAgentVersion( std::string *_Ver ) const
{
//    if ( !_Ver )
//        return ;
//
//    FILE *f = popen( "pod-agent --version", "r" );
//    if ( !f )
//        return ;
//
//    char * line = NULL;
//    size_t len = 0;
//    stringstream ss;
//    while ( getline( &line, &len, f ) != -1 )
//    {
//        ss << line;
//    }
//    if ( line )
//        free( line );
//    pclose( f );
//    ss << '\n';
//    *_Ver = ss.str();
}
