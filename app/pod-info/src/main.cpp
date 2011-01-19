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
        m_connectionString( false ),
        m_listWNs( false )
    {
    }

    bool m_version;
    bool m_connectionString;
    bool m_listWNs;
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
    ( "list,l", "List all available PROOF workers" )
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
        _options->m_connectionString = true;

    if( vm.count( "list" ) )
        _options->m_listWNs = true;

    return true;
}
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
        ss << "PoD server is NOT found, probably is not running.";
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
string connectionString( const pod_info::CServer &_srv )
{
    PROOFAgent::SHostInfoCmd srvHostInfo;
    try
    {
        _srv.getSrvHostInfo( &srvHostInfo );
    }
    catch( exception &_e )
    {
        string msg;
        msg += "Can't connect to PoD server. Please check that the server is running.\n";
        msg += _e.what();
        throw runtime_error( msg );
    }
    ostringstream ss;
    ss
            << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
            << srvHostInfo.m_proofPort;

    return ( ss.str() );
}
//=============================================================================
string listWNs( const pod_info::CServer &_srv )
{
    StringVector_t lst;
    try
    {
        _srv.getListOfWNs( &lst );
    }
    catch( exception &_e )
    {
        string msg;
        msg += "Can't connect to PoD server. Please check that the server is running.\n";
        msg += _e.what();
        throw runtime_error( msg );
    }
    stringstream ss;
    std::ostream_iterator< std::string > output( ss, "\n" );
    std::copy( lst.begin(), lst.end(), output );
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
        }

        // Show list of workers
        if( options.m_listWNs )
        {
            cout << listWNs( srv ) << endl;
        }


    }
    catch( exception& e )
    {
        cerr << PROJECT_NAME << ": error: " << e.what() << endl;
        return 1;
    }


    return 0;
}
