/************************************************************************/
/**
 * @file Test_MiscUtils.cpp
 * @brief Unit tests of MiscCommon
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-03-10
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// BOOST: tests
#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

// STD
#include <string>

// Our
#include "MiscUtils.h"

using namespace MiscCommon;
using namespace std;

void test_smart_append();
void test_replace();
void test_to_lower();
void test_to_upper();


test_suite* init_unit_test_suite( int, char* [] )
{
    test_suite * test = BOOST_TEST_SUITE( "Unit tests of MiscCommon" );

    test->add( BOOST_TEST_CASE( &test_smart_append ), 0 );
    test->add( BOOST_TEST_CASE( &test_replace ), 0 );
    test->add( BOOST_TEST_CASE( &test_to_lower ), 0 );
    test->add( BOOST_TEST_CASE( &test_to_upper ), 0 );

    return test;
}



void test_smart_append()
{
    const string sTempl("/Test1/Test/");
    const string sTempl2("Test1/Test/");

    {
        string sVal = "/Test1/Test/";
        smart_append( &sVal, '/' );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "/Test1/Test";
        smart_append( &sVal, '/' );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "Test1/Test";
        smart_append( &sVal, '/' );
        BOOST_CHECK( sTempl2 == sVal );
    }
}

void test_replace()
{
    const string sTempl("Test_HELLO/Tset");

    {
        string sVal = "%1_HELLO/Tset";
        replace<string>( &sVal, "%1", "Test");
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "Test_HELLO/%1";
        replace<string>( &sVal, "%1", "Tset");
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "Test_%1/Tset";
        replace<string>( &sVal, "%1", "HELLO");
        BOOST_CHECK( sTempl == sVal );
    }

}

void test_to_lower()
{
    const string sTempl("test_4hello");

    {
        string sVal = "TesT_4HEllO";
        to_lower( sVal );
        BOOST_CHECK( sTempl == sVal );
    }
}

void test_to_upper()
{
    const string sTempl("TEST2_HELLO");

    {
        string sVal = "TesT2_HEllo";
        to_upper( sVal );
        BOOST_CHECK( sTempl == sVal );
    }
}

void test_trim()
{
    const string sTempl("TEST2_HELLO");

    {
        string sVal = " TEST2_HELLO ";
        trim<string>( &sVal, " " );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "/TEST2_HELLO";
        trim<string>( &sVal, "/" );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "TEST2_HELLO/";
        trim<string>( &sVal, "/" );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "TEST2_HELLOO";
        trim<string>( &sVal, "O" );
        BOOST_CHECK( sTempl == sVal );
    }

    {
        string sVal = "HELLOTEST2_HELLOHELLO";
        trim<string>( &sVal, "HELLO" );
        BOOST_CHECK( sTempl == sVal );
    }
}
