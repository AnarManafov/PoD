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
// pod-info
#include "version.h"
#include "Environment.h"
#include "Server.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;
//=============================================================================
struct SOptions
{
    SOptions(): 
    m_version( false ),
    m_connectionString(false)
    {
    }

    bool m_version;
    bool m_connectionString;
};
//=============================================================================
// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], SOptions *_options ) throw( exception )
{
    if( !_options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "connection_string,c", "Show PROOF connection string" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    if( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if( vm.count( "version" ) )
    {
        _options->m_version = true;
        return true;
    }
    if( vm.count( "connection_string" ) )
    {
        _options->m_connectionString = true;
        return true;
    }

    return true;
}
//=============================================================================
string version( const CEnvironment &_env, const pod_info::CServer &_srv )
{
    PROOFAgent::SHostInfoCmd srvHostInfo;
    _srv.getSrvHostInfo( &srvHostInfo );

    ostringstream ss;
    ss
            << "PoD location: " << _env.PoDPath() << "\n"
            << "Local Version: " << _env.version() << "\n";

    ss
            << "Server PoD location: "
            << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
            << srvHostInfo.m_PoDPath << "\n"
            << "Server Version: " << srvHostInfo.m_version;

    return ( ss.str() );
}
//=============================================================================
string connectionString( const pod_info::CServer &_srv )
{
    PROOFAgent::SHostInfoCmd srvHostInfo;
    _srv.getSrvHostInfo( &srvHostInfo );
    
    ostringstream ss;
    ss
    << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
    << srvHostInfo.m_proofPort;
    
    return ( ss.str() );
}
//=============================================================================
int main( int argc, char *argv[] )
{
    CEnvironment env;
    try
    {
        env.init();

        SOptions options;
        if( !parseCmdLine( argc, argv, &options ) )
            return 0;

        string srvHost;
        unsigned int srvPort( 0 );
        // try to connect to PoD server
        if( env.isLocalServer() )
        {
            srvHost = env.serverHost();
            srvPort = env.serverPort();
        }
        else
        {
            // TODO: Not implemented yet
        }
        
        if( 0 == srvPort)
            throw runtime_error( "Can't connect to PoD server. Please check that the server is running." );
        
        pod_info::CServer srv( srvHost, srvPort );

        // Show version information
        if( options.m_version )
        {
            cout << version( env, srv ) << endl;
            return 0;
        }
        
        // Show connection string
        if( options.m_connectionString )
        {
            cout << connectionString( srv ) << endl;
            return 0;
        }

    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": error: " << e.what() << endl;
        return 1;
    }


    return 0;
}
