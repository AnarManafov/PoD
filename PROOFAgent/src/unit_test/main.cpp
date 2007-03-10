
// BOOST: tests
#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

// Our
#include "INet.h"

using namespace MiscCommon::INet;

void test_smart_counter();

        
test_suite* init_unit_test_suite( int, char* [] )
{
   test_suite * test = BOOST_TEST_SUITE( "Unit tests of PROOFAgent" );

   // test->add( BOOST_TEST_CASE( &tf_smart_socket ), 0 );

    return test;
}



void test_smart_counter()
{
    /*{
        smart_socket s1;
        BOOST_CHECK( s1.count() == 0 );
    }

    {
        smart_socket s2( AF_INET, SOCK_STREAM, 0 );
        BOOST_CHECK( s1.count() == 0 );
        int Socket = s2;
        s2.add_ref();
            ::close(Socket);
        BOOST_CHECK( s2.count() == 0 );
    }*/
}


