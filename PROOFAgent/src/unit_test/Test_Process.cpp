/************************************************************************/
/**
 * @file Test_Process.cpp
 * @brief Unit tests of Process.h
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-10-31
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST: tests
#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

// MiscCommon
#include "Process.h"

using namespace MiscCommon;
using namespace std;

void test_smart_append();
void test_CProcStatus();


test_suite* init_unit_test_suite( int, char* [] )
{
    test_suite * test = BOOST_TEST_SUITE( "Unit tests of Process.h" );

    test->add( BOOST_TEST_CASE( &test_CProcStatus ), 0 );

    return test;
}



void test_CProcStatus()
{
  CProcStatus p;
  pid_t pid( ::getpid() );
  p.Open( pid );
  BOOST_CHECK( p.GetValue("Name") == "UT_Process" );
  
  BOOST_CHECK( p.GetValue("NAME") == "UT_Process" );
  
  stringstream ss_pid;
  ss_pid << pid;
  BOOST_CHECK( p.GetValue("Pid") == ss_pid.str() );
  
}
