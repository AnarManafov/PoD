/************************************************************************/
/**
 * @file test_oge.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-10-13
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
#include <iostream>
#include <iterator>
// MiscCommon
#include "def.h"
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN    // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
// pod-console
#include "version.h"
#include "OgeMng.h"
//=============================================================================
using namespace std;
using namespace oge_plug;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE( test_oge );
//=============================================================================
size_t g_jobsCount = 3;
//=============================================================================
BOOST_AUTO_TEST_CASE( test_oge_allqueues )
{
    cout << "Getting a list of available queues" << endl;
    COgeMng mng;

    COgeMng::queueInfoContainer_t queues;
    mng.getQueues( &queues );

    BOOST_REQUIRE( !queues.empty() );

    copy( queues.begin(), queues.end(),
          ostream_iterator<SQueueInfo>( cout, "\n" ) );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_oge_submitjob )
{
    // create a test script
    char tmpname[] = "/tmp/oge_test_job.XXXXXX";
    int tmpfd( 0 );
    BOOST_REQUIRE(( tmpfd = mkstemp( tmpname ) ) >= 0 );
    FILE *file;
    BOOST_REQUIRE(( file = fdopen( tmpfd, "w" ) ) != NULL );
    fprintf( file, "#! /usr/bin/env bash\n" );
    fprintf( file, "echo \"Test\"\n" );
    fclose( file );

    COgeMng mng;

    // check that submit works
    // submit a job to a default queue
    COgeMng::jobArray_t ids = mng.jobSubmit( tmpname, "", g_jobsCount );
    BOOST_REQUIRE( !ids.empty() );

    // we need to sleep a bit. Otherwise we could be too fast asking for status, than OGE registers a job
    sleep( 2 );

    cout << "Fake parent ID: " << ids[0] << endl;
    while ( ids.size() > 1 )
    {
        COgeMng::jobArray_t::iterator iter = ids.begin() + 1;
        COgeMng::jobArray_t::iterator iter_end = ids.end();
        for ( ; iter != iter_end; ++iter )
        {
            // get job's status
            int status = mng.jobStatus( *iter );
            if ( mng.isJobComplete( status ) )
            {
                cout << "Array jobs ID: " << *iter << "is DONE" << endl;
                ids.erase( iter );
                break;
            }
            cout << "Array jobs ID: " << *iter << "\t";
            string strStatus( mng.status2string( status ) );
            cout << "Status: " << strStatus << endl;
            // we compare status with some unknown (99999) value
            BOOST_REQUIRE( mng.status2string( 99999 ) != strStatus );
        }
        sleep( 1 );
    }
    // TODO: delete the script, even in case of an error
    // remove the test script
    unlink( tmpname );
}

BOOST_AUTO_TEST_SUITE_END();
