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
size_t g_jobsCount = 2;
//=============================================================================
BOOST_AUTO_TEST_CASE( test_oge_submitjob )
{
    // create a test script
    char tmpname[] = "/tmp/pbs_script.XXXXXX";
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

    COgeMng::jobArray_t::const_iterator iter = ids.begin() + 1;
    COgeMng::jobArray_t::const_iterator iter_end = ids.end();
    for( ; iter != iter_end; ++iter )
    {
        cout << "Array jobs ID: " << *iter << endl;
    }



//    // we need to sleep a bit. Otherwise we could be too fast asking for status, than OGE registers a job
//    sleep( 2 );
//
//    cout << "Fake parent ID: " << ids[0] << endl;
//    COgeMng::jobArray_t::const_iterator iter = ids.begin() + 1;
//    COgeMng::jobArray_t::const_iterator iter_end = ids.end();
//    for ( ; iter != iter_end; ++iter )
//    {
//        cout << "Array jobs ID: " << *iter << endl;
//
//        // get job's status
//        string status = mng.jobStatus( *iter );
//        cout << "Status: " << status << endl;
//        BOOST_REQUIRE( status.size() == 1 );
//    }

    // TODO: delete the script, even in case of an error
    // remove the test script
    //unlink( tmpname );
}
////=============================================================================
//BOOST_AUTO_TEST_CASE( test_pbs_alljobs )
//{
//    cout << "Check status of all jobs" << endl;
//    COgeMng mng;
//
//    COgeMng::jobInfoContainer_t info;
//    mng.jobStatusAllJobs( &info );
//
//    BOOST_REQUIRE( !info.empty() );
//
//    COgeMng::jobInfoContainer_t::const_iterator iter = info.begin();
//    COgeMng::jobInfoContainer_t::const_iterator iter_end = info.end();
//    for ( ; iter != iter_end ; ++iter )
//    {
//        cout << iter->first << " has status \""
//        << COgeMng::jobStatusToString( iter->second.m_status ) << "\"" << endl;
//    }
//}
////=============================================================================
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

BOOST_AUTO_TEST_SUITE_END();