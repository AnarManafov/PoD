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
#include "ServerInfo.h"

// OUR
#include "Process.h"
#include "def.h"

using namespace std;
using namespace MiscCommon;

struct SFindName: public binary_function< CProcList::ProcContainer_t::value_type, string, bool >
{
    bool operator() ( CProcList::ProcContainer_t::value_type _pid, const string &_Name ) const
    {
        CProcStatus p;
        p.Open( _pid );
        return ( p.GetValue( "Name" ) == _Name );
    }
};

pid_t CServerInfo::IsRunning( const string &_Srv ) const
{
    CProcList::ProcContainer_t pids;
    CProcList::GetProcList( &pids );

    CProcList::ProcContainer_t::const_iterator iter = find_if( pids.begin(), pids.end(), bind2nd(SFindName(), _Srv));
    return ( pids.end() != iter ? *iter : 0 );
}

pid_t CServerInfo::IsXROOTDRunning() const
{
    return IsRunning( "xrootd" );
}

pid_t CServerInfo::IsPROOFAgentRunning() const
{
    return IsRunning( "proofagent" );
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
    << (pid ? "running" : "not running");

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
    << (pid ? "running" : "not running");

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
    while ( getline(&line, &len, f) != -1 )
    {
        ss << line << "\n";
    }
    if ( line )
        free(line);
    pclose(f);
    *_Ver = ss.str();
}
