/************************************************************************/
/**
 * @file main.cpp
 * @brief main file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-06-30
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/program_options/cmdline.hpp>
// STD
#include <iostream>
#include <fstream>
#include <string>
// MiscCommon
#include "PoDUserDefaultsOptions.h"
#include "BOOSTHelper.h"
#include "SysHelper.h"
//
#include "version.h"

using namespace PoD;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

void printVersion()
{
    cout
            << PROJECT_NAME << " v" << PROJECT_VERSION_STRING << "\n"
            << "PoD configuration file" << " v." << USER_DEFAULTS_CFG_VERSION  << "\n"
            << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}

// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], SPoDUserDefaultsOptions_t *_Options ) throw( exception )
{
    if( !_Options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "config,c", bpo::value<string>(), "PoD user-defaults configuration file" )
    ( "key", bpo::value<string>(), "Get a value for the given key" )
    ( "default,d", "Generate a default PoD configuration file" )
    ( "force,f", "If the destination file exists, remove it and create a new file, without prompting for confirmation" )
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
        printVersion();
        return false;
    }

    boost_hlp::option_dependency( vm, "key", "config" );
    boost_hlp::option_dependency( vm, "default", "config" );
    boost_hlp::conflicting_options( vm, "default", "key" );
    boost_hlp::conflicting_options( vm, "force", "key" );

    CPoDUserDefaults user_defaults;

    if( vm.count( "default" ) )
    {
        cout << "Generating a default PoD configuration file..." << endl;

        string filename( vm["config"].as<string>() );
        if( MiscCommon::does_file_exists( filename ) && !vm.count( "force" ) )
            throw runtime_error( "Error: Destination file exists. Please use -f options to overwrite it." );

        ofstream f( filename.c_str() );
        if( !f.is_open() )
        {
            string s( "Can't open file " );
            s += filename;
            s += " for writing.";
            throw runtime_error( s );
        }

        f << "# PoD user defaults\n"
          << "# version: " << USER_DEFAULTS_CFG_VERSION << "\n"
          << "#\n"
          << "# Please use PoD User's Manual to find out more details on\n"
          << "# keys and values of this configuration file.\n"
          << "# PoD User's Manual can be found in $POD_LOCATION/doc folder or\n"
          << "# by the following address: http://pod.gsi.de/documentation.html\n";
        CPoDUserDefaults::printDefaults( f );
        cout << "Generating a default PoD configuration file - DONE." << endl;
        return false;
    }

    if( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }
    else
    {
        user_defaults.init( vm["config"].as<string>() );
    }

    if( vm.count( "key" ) )
    {
        cout << user_defaults.getValueForKey( vm["key"].as<string>() ) << endl;
    }

    return true;
}

int main( int argc, char *argv[] )
{
    // Command line parser
    SPoDUserDefaultsOptions_t Options;
    try
    {
        if( !parseCmdLine( argc, argv, &Options ) )
            return 0;
    }
    catch( exception& e )
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return 1;
    }


    return 0;
}
