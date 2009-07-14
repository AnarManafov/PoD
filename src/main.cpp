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

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
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

using namespace PoD;
using namespace std;
namespace bpo = boost::program_options;
namespace boost_hlp = MiscCommon::BOOSTHelper;

void printVersion()
{
    // TODO: make VERSION to be taken from the build
    cout << "TODO: print version information" << endl;
}

// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], SPoDUserDefaultsOptions_t *_Options ) throw( exception )
{
    if ( !_Options )
        throw runtime_error( "Internal error: options' container is empty." );

    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "produce help message" )
    ( "version,v", "version information" )
    ( "config,c", bpo::value<string>(), "PoD user-defaults configuration file" )
    ( "key", bpo::value<string>(), "get a value for the given key" )
    ;

    // Parsing command-line
    bpo::variables_map vm;
    bpo::store( bpo::command_line_parser( _Argc, _Argv ).options( visible ).run(), vm );
    bpo::notify( vm );

    if ( vm.count( "help" ) || vm.empty() )
    {
        cout << visible << endl;
        return false;
    }
    if ( vm.count( "version" ) )
    {
        printVersion();
        return false;
    }

    boost_hlp::option_dependency( vm, "key", "config" );

    CPoDUserDefaults user_defaults;

    if ( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }
    else
    {
        user_defaults.init( vm["config"].as<string>() );
    }

    if ( vm.count( "key" ) )
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
        if ( !parseCmdLine( argc, argv, &Options ) )
            return 0;
    }
    catch ( exception& e )
    {
        // TODO: Log me!
        cerr << e.what() << endl;
        return 1;
    }


    return 0;
}
