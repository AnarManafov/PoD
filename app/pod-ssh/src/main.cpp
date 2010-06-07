/************************************************************************/
/**
 * @file main.cpp
 * @brief Implementation of the "Main" function
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-05-17
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI, Scientific Computing group. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
// STD
#include <iostream>
#include <fstream>
#include <stdexcept>
// pod-ssh
#include "version.h"
#include "config.h"

using namespace std;
namespace bpo = boost::program_options;
//=============================================================================
void printVersion()
{
    cout << PROJECT_NAME << " v." << PROJECT_VERSION_STRING << "\n"
         << "Report bugs/comments to A.Manafov@gsi.de" << endl;
}
//=============================================================================
// Command line parser
bool parseCmdLine( int _Argc, char *_Argv[], bpo::variables_map *_vm )
{
    // Generic options
    bpo::options_description visible( "Options" );
    visible.add_options()
    ( "help,h", "Produce help message" )
    ( "version,v", "Version information" )
    ( "config,c", bpo::value<string>(), "PoD's ssh plug-in configuration file" )
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

    if( !vm.count( "config" ) )
    {
        cout << visible << endl;
        throw runtime_error( "You need to specify a configuration file at least." );
    }

    _vm->swap( vm );
    return true;
}
//=============================================================================
int main( int argc, char *argv[] )
{
    bpo::variables_map vm;
    try
    {
        if( !parseCmdLine( argc, argv, &vm ) )
            return 0;
        ifstream f( vm["config"].as<string>().c_str() );
        if( !f.is_open() )
        {
            string msg( "Error: can't open configuration file \"" );
            msg += vm["config"].as<string>();
            msg += "\"";
            throw runtime_error( msg );
        }

        CConfig config;
        config.readFrom( f );
    }
    catch( exception& e )
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
