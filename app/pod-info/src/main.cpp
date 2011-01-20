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
        m_listWNs( false ),
        m_countWNs( false ),
        m_status( false ),
        m_debug( false )
    {
    }
    bool operator== ( const SOptions &_val )
    {
        return ( m_version == _val.m_version &&
                 m_connectionString == _val.m_connectionString &&
                 m_listWNs == _val.m_listWNs &&
                 m_countWNs == _val.m_countWNs &&
                 m_status == _val.m_status &&
                 m_debug == _val.m_debug );
    }

    bool m_version;
    bool m_connectionString;
    bool m_listWNs;
    bool m_countWNs;
    bool m_status;
    bool m_debug;
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
    ( "version,v", bpo::bool_switch( &( _options->m_version ) ), "Version information" )
    ( "debug,d", bpo::bool_switch( &( _options->m_debug ) ), "Show debug messages" )
    ( "connection_string,c", bpo::bool_switch( &( _options->m_connectionString ) ), "Show PROOF connection string" )
    ( "list,l", bpo::bool_switch( &( _options->m_listWNs ) ), "List all available PROOF workers" )
    ( "number,n", bpo::bool_switch( &( _options->m_countWNs ) ), "Report a number of currently available PROOF workers" )
    ( "status,s", bpo::bool_switch( &( _options->m_status ) ), "Show status of PoD server" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    // we need an empty struct to check the case when user don't provide any argument
    SOptions s;
    if( vm.count( "help" ) || ( s == *_options ) )
    {
        cout << visible << endl;
        return false;
    }

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
void srvPoDStatus( string *_status, string *_con_string,
                   const pod_info::CServer &_srv, const SOptions &_opt )
{
    PROOFAgent::SHostInfoCmd srvHostInfo;
    try
    {
        _srv.getSrvHostInfo( &srvHostInfo );
    }
    catch( exception &_e )
    {
        string msg;
        msg += "PoD server is NOT found.\n";
        if( _opt.m_debug )
            msg += _e.what();
        throw runtime_error( msg );
    }

    if( _status )
    {
        ostringstream ss;
        ss
                << "XPROOFD [" << srvHostInfo.m_xpdPid << "] port: " << srvHostInfo.m_xpdPort
                << "\nPoD agent [" << srvHostInfo.m_agentPid << "] port: " << srvHostInfo.m_agentPort;
        *_status = ss.str();
    }

    if( _con_string )
    {
        ostringstream ss;
        ss
                << srvHostInfo.m_username << "@" << srvHostInfo.m_host << ":"
                << srvHostInfo.m_xpdPort;
        *_con_string = ss.str();
    }

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

        // PoD Server status
        if( options.m_status || options.m_connectionString )
        {
            string status;
            string con_string;
            srvPoDStatus( &status, &con_string, srv, options );
            if( options.m_status )
            {
                cout << status << endl;
            }
            if( options.m_connectionString )
            {
                cout << con_string << endl;
            }
        }

        // list of and a number of available WNs
        if( options.m_countWNs || options.m_listWNs )
        {
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
        cerr << PROJECT_NAME << ": error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
