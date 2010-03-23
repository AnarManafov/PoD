/************************************************************************/
/**
 * @file test_submit.cpp
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

BOOST_AUTO_TEST_SUITE( test_submit );
//=============================================================================
const string g_pbsScript = "./test.pbs";
//=============================================================================
BOOST_AUTO_TEST_CASE( test_submit_0 )
{
    CPbsMng mng;

    CPbsMng::jobID_t id = mng.jobSubmit( g_pbsScript, "batch", 2, "./" );
    BOOST_REQUIRE( mng.isValid( id ) );
}
BOOST_AUTO_TEST_SUITE_END();
