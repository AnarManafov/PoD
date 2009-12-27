/************************************************************************/
/**
 * @file Test_ProtocolCommands.cpp
 * @brief Unit tests of ProtocolCommands
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-26
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_AUTO_TEST_MAIN    // Boost 1.33
#define BOOST_TEST_MAIN
//#include <boost/test/unit_test.hpp>
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test::test_suite;
//using namespace boost::unit_test;
// pod-agent
#include "ProtocolCommands.h"
//=============================================================================
using namespace std;
using namespace PROOFAgent;
////=============================================================================
//void test_SVersionCmd();
//void test_SVersionCmd_BadData();
//void test_SHostInfoCmd();
//void test_SHostInfoCmd_BadData();
////=============================================================================
//test_suite* init_unit_test_suite( int, char* [] )
//{
//    test_suite * test = BOOST_TEST_SUITE( "Unit tests of ProtocolCommands (pod-agent)" );
//
//    test->add( BOOST_TEST_CASE( &test_SVersionCmd ), 0 );
//    test->add( BOOST_TEST_CASE( &test_SVersionCmd_BadData ), 0 );
//    test->add( BOOST_TEST_CASE( &test_SHostInfoCmd ), 0 );
//    test->add( BOOST_TEST_CASE( &test_SHostInfoCmd_BadData ), 0 );
//
//    return test;
//}
BOOST_AUTO_TEST_SUITE( pp );
//=============================================================================
BOOST_AUTO_TEST_CASE(test_SVersionCmd)
{
	BOOST_MESSAGE("....");
    SVersionCmd a;
    a.m_version = 34;
    MiscCommon::BYTEVector_t data;
    a.convertToData( &data );

    SVersionCmd b;
    b.convertFromData( data );

    BOOST_CHECK( a == b );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_SVersionCmd_BadData)
{
    SVersionCmd a;
    a.m_version = 34;
    MiscCommon::BYTEVector_t data;
    a.convertToData( &data );

    // making data array shorter
    data.resize( data.size() - 2 );

    SVersionCmd b;
    BOOST_CHECK_THROW( b.convertFromData( data ), runtime_error );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_SHostInfoCmd)
{
    SHostInfoCmd a;
    a.m_username = "testuser";
    a.m_host = "test.host.de";
    a.m_proofPort = 129;
    MiscCommon::BYTEVector_t data;
    a.convertToData( &data );

    SHostInfoCmd b;
    b.convertFromData( data );

    BOOST_CHECK( a == b );
}
//=============================================================================
BOOST_AUTO_TEST_CASE(test_SHostInfoCmd_BadData)
{
    SHostInfoCmd a;
    a.m_username = "testuser";
    a.m_host = "test.host.de";
    a.m_proofPort = 129;
    MiscCommon::BYTEVector_t data;
    a.convertToData( &data );

    // making data array shorter
    data.resize( data.size() - 2 );

    SHostInfoCmd b;
    BOOST_CHECK_THROW( b.convertFromData( data ), runtime_error );
}
BOOST_AUTO_TEST_SUITE_END();
