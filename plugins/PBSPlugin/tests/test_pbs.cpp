/************************************************************************/
/**
 * @file test_pbs.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-03-22
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
#include <iostream>
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN    // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
// pod-console
#include "version.h"
#include "PbsMng.h"
//=============================================================================
using namespace std;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE( test_pbs );
//=============================================================================
size_t g_jobsCount = 2;
//=============================================================================
BOOST_AUTO_TEST_CASE( test_pbs_0 )
{
    // create a test script
    char tmpname[] = "/tmp/pbs_script.XXXXXX";
    int tmpfd( 0 );
    BOOST_REQUIRE(( tmpfd = mkstemp( tmpname ) ) >= 0 );
    FILE *file;
    BOOST_REQUIRE(( file = fdopen( tmpfd, "w" ) ) != NULL );
    fprintf( file, "#! /usr/bin/env bash\n" );
    fprintf( file, "echo \"Test\"\n" );

    CPbsMng mng;

    // check that submit works
    CPbsMng::jobArray_t ids = mng.jobSubmit( tmpname, "batch", g_jobsCount, "./" );
    BOOST_REQUIRE( !ids.empty() );

    cout << "Fake parent ID: " << ids[0] << endl;
    CPbsMng::jobArray_t::const_iterator iter = ids.begin() + 1;
    CPbsMng::jobArray_t::const_iterator iter_end = ids.end();
    for ( ; iter != iter_end; ++iter )
    {
        cout << "Array jobs IDs: " << *iter << endl;

        // get job's status
        mng.jobStatus( *iter );
    }

    // remove the test script
    unlink( tmpname );
}
BOOST_AUTO_TEST_SUITE_END();
