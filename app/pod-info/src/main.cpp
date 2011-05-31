/************************************************************************/
/**
 * @file main.cpp
 * @brief main file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2011-01-17
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2011 GSI, Scientific Computing devision. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
// MiscCommon
#include "BOOSTHelper.h"
#include "Process.h"
#include "SysHelper.h"
#include "PoDUserDefaultsOptions.h"
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
#include "Options.h"
#include "SrvInfo.h"
#include "SSHTunnel.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
enum EPoDServerType
{
    // PoD Server can't be found
    SrvType_Unknown,
    // a local PoD server.
    SrvType_Local,
    // a remote PoD server
    SrvType_Remote
};
//=============================================================================
string version( const CEnvironment &_env, const pod_info::CServer &_srv )
{
    bool noServer( false );
    PROOFAgent::SHostInfoCmd srvHostInfo;
    try
    {
        _srv.getSrvHostInfo( &srvHostInfo );
    }
    catch( ... )
    {
        noServer = true;
    }
    ostringstream ss;
    ss
            << "PoD location: " << _env.PoDPath() << "\n"
            << "Local Version: " << _env.version() << "\n";

    if( noServer )
    {
        ss << "PoD server is NOT found.";
    }
    else
    {
        ss
                << "Server PoD location: "
                << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
                << srvHostInfo.m_PoDPath << "\n"
                << "Server Version: " << srvHostInfo.m_version;
    }
    return ( ss.str() );
}
//=============================================================================
size_t listWNs( string *_output, const pod_info::CServer &_srv,
                const SOptions &_opt )
{
    PROOFAgent::SWnListCmd lst;
    try
    {
        _srv.getListOfWNs( &lst );
    }
    catch( exception &_e )
    {
        string msg;
        msg += "PoD server is NOT found.\n";
        if( _opt.m_debug )
            msg += _e.what();
        throw runtime_error( msg );
    }
    if( _output )
    {
        stringstream ss;
        std::ostream_iterator< std::string > output( ss, "\n" );
        std::copy( lst.m_container.begin(), lst.m_container.end(), output );
        *_output = ss.str();
    }
    return ( lst.m_container.size() );
}
//=============================================================================
void retrieveRemoteServerInfo( const SOptions &_opt,
                               const string &_destinationFile )
{
    // delete first the remote srv info file
    unlink( _destinationFile.c_str() );

    StringVector_t arg;
    string sourceFile( _opt.m_remotePath );
    sourceFile += "etc/server_info.cfg";
    arg.push_back( "-s" + sourceFile );
    arg.push_back( "-l " + _opt.m_sshConnectionStr );
    arg.push_back( "-f " + _destinationFile );
    if( _opt.m_debug )
        arg.push_back( "-d" );
    if( _opt.m_batchMode )
        arg.push_back( "-b" );
    string cmd( "$POD_LOCATION/bin/private/pod-remote-srv-info" );
    smart_path( &cmd );
    string stdout;
    do_execv( cmd, arg, 60, NULL );
    if( !does_file_exists( _destinationFile ) )
    {
        stringstream ss;
        ss << "Remote PoD server is NOT running.";
        throw runtime_error( ss.str() );
    }
}
//=============================================================================
int main( int argc, char *argv[] )
{
    CEnvironment env;
    EPoDServerType srvType( SrvType_Unknown );

    try
    {
        env.init();

        SOptions options;
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        // An SSH tunnel object
        CSSHTunnel sshTunnel;

        // Short info about locals
        if( options.m_xpdPid )
        {
            CSrvInfo srvInfo( &env );
            srvInfo.getInfo();
            if( 0 == srvInfo.xpdPid() )
                return 1;

            cout << srvInfo.xpdPid() << endl;
            return 0;
        }
        if( options.m_xpdPort )
        {
            CSrvInfo srvInfo( &env );
            srvInfo.getInfo();
            if( 0 == srvInfo.xpdPort() )
                return 1;

            cout << srvInfo.xpdPort() << endl;
            return 0;
        }
        if( options.m_agentPid )
        {
            CSrvInfo srvInfo( &env );
            srvInfo.getInfo();
            if( 0 == srvInfo.agentPid() )
                return 1;

            cout << srvInfo.agentPid() << endl;
            return 0;
        }
        if( options.m_agentPort )
        {
            if( !env.processServerInfoCfg() )
                return 1;

            CSrvInfo srvInfo( &env );
            srvInfo.getInfo();
            if( 0 == srvInfo.agentPort() )
                return 1;

            cout << srvInfo.agentPort() << endl;
            return 0;
        }

        // Check PoD server's Type
        srvType = ( options.m_sshConnectionStr.empty() ) ? SrvType_Local : SrvType_Remote;

        // if the type of the server is remote, than we need to get a remote
        // server info file
        string srvHost;
        // use SSH to retrieve server_info.cfg
        if( SrvType_Remote == srvType )
        {
            if( options.m_debug )
            {
                cout << "Trying: remote PoD server" << endl;
            }
            string outfile( env.remoteSrvInfoFile() );
            retrieveRemoteServerInfo( options, outfile );
            env.processServerInfoCfg( &outfile );
            // now we can delete the remote server file
            // we can't reuse it in next sessions, since the PoD port could change
            unlink( outfile.c_str() );

            // we tunnel the connection to PoD server
            sshTunnel.setPidFile( env.getTunnelPidFile() );
            sshTunnel.create( env, options );
            // if we tunnel pod-agent's port, than we need to connect to a localhost
            srvHost = "localhost";
        }
        else
        {
            if( options.m_debug )
            {
                cout
                        << "Trying: local PoD server"
                        << "Server Info: " << env.localSrvInfoFile() << endl;
            }
            // Process a local server-info.
            // If "--version" is given, than we don't throw,
            // because we need a version info in anyway, even without any server
            if( !env.processServerInfoCfg() && !options.m_version )
            {
                string msg;
                msg += "PoD server is NOT running.";
                if( options.m_debug )
                {
                    msg += "\nCan't process server info: ";
                    msg += env.localSrvInfoFile();
                }
                throw runtime_error( msg );
            }
            srvHost = env.serverHost();
        }

        // Show version information
        if( options.m_version )
        {
            pod_info::CServer srv( srvHost, env.serverPort() );
            cout << version( env, srv ) << endl;
            return 0;
        }

        // PoD Server status
        if( options.m_status || options.m_connectionString )
        {
            CSrvInfo srvInfo( &env );
            string localHostName;
            get_hostname( &localHostName );
            // If we run locally it could mean, we run on a shared file system as well.
            // We therefore need to check, whether we run on the same machine as the server
            if( srvType == SrvType_Local && localHostName == srvHost )
            {
                srvInfo.getInfo();
            }
            else
            {
                pod_info::CServer srv( srvHost, env.serverPort() );
                srvInfo.getInfo( &srv );
            }

            if( options.m_status )
            {
                srvInfo.printInfo( cout );
                if( srvInfo.getStatus() != CSrvInfo::srvStatus_OK )
                {
                    return srvInfo.getStatus();
                }
            }
            if( options.m_connectionString )
            {
                srvInfo.printConnectionString( cout );
            }
        }

        // list of and a number of available WNs
        if( options.m_countWNs || options.m_listWNs )
        {
            pod_info::CServer srv( srvHost, env.serverPort() );
            string lst;
            size_t n = listWNs(( options.m_listWNs ? &lst : NULL ), srv, options );

            if( options.m_countWNs )
            {
                cout << n << endl;
            }
            if( options.m_listWNs )
            {
                cout << lst;
                cout.flush();
            }
        }
    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }
    return 0;
}
