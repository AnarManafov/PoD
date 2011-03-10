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
#include "logEngine.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
void version()
{
    cout << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
int main( int argc, char *argv[] )
{
    CLogEngine slog;
    CEnvironment env;

    try
    {
        SOptions options;
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        // Show version information
        if( options.m_version )
        {
            version();
            return 0;
        }

        env.init();
        slog.start( env.getlogEnginePipeName() );

        if( options.m_start )
        {
            stringstream ss;
            ss
                    << "Starting a remote PoD server on "
                    << options.m_sshConnectionStr << '\n';
            slog( ss.str() );

        }

//        slog( "Start --\n" );

//        // An SSH tunnel object
//        CSSHTunnel sshTunnel;
//
//        // Check PoD server's Type
//        srvType = ( options.m_sshConnectionStr.empty() ) ? SrvType_Local : SrvType_Remote;
//
//        // if the type of the server is remote, than we need to get a remote
//        // server info file
//        string srvHost;
//        // use SSH to retrieve server_info.cfg
//        if( SrvType_Remote == srvType )
//        {
//            if( options.m_debug )
//            {
//                cout << "Trying: remote PoD server" << endl;
//            }
//            string outfile( env.remoteSrvInfoFile() );
//            retrieveRemoteServerInfo( options, outfile );
//            env.processServerInfoCfg( &outfile );
//            // now we can delete the remote server file
//            // we can't reuse it in next sessions, since the PoD port could change
//            unlink( outfile.c_str() );
//
//            // we tunnel the connection to PoD server
//            sshTunnel.setPidFile( env.getTunnelPidFile() );
//            sshTunnel.create( env, options );
//            // if we tunnel pod-agent's port, than we need to connect to a localhost
//            srvHost = "localhost";
//        }
//        else
//        {
//            if( options.m_debug )
//            {
//                cout
//                        << "Trying: local PoD server"
//                        << "Server Info: " << env.localSrvInfoFile() << endl;
//            }
//            // Process a local server-info.
//            // If "--version" is given, than we don't throw,
//            // because we need a version info in anyway, even without any server
//            if( !env.processServerInfoCfg() && !options.m_version )
//            {
//                string msg;
//                msg += "PoD server is NOT running.";
//                if( options.m_debug )
//                {
//                    msg += "\nCan't process server info: ";
//                    msg += env.localSrvInfoFile();
//                }
//                throw runtime_error( msg );
//            }
//            srvHost = env.serverHost();
//        }

    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": " << e.what() << endl;
        return 1;
    }
    return 0;
}
