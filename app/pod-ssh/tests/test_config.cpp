/************************************************************************/
/**
 * @file test_config.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-06-07
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
// pod-ssh
#include "version.h"
#include "config.h"
//=============================================================================
using namespace std;
using boost::unit_test::test_suite;

BOOST_AUTO_TEST_SUITE( pod_ssh_config );
//=============================================================================

//=============================================================================
BOOST_AUTO_TEST_CASE( test_readconfig )
{
    CConfig config;

    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, 4\n"
       << "r2, anar@lxi001.gsi.de,,2\n"
       << "125, anar@lxg0055.gsi.de, -p22, 8\n";

    config.readFrom( ss );

    configRecords_t recs( config.getRecords() );
    BOOST_REQUIRE( !recs.empty() );

    // Checking record #1
    SConfigRecord rec;
    rec.m_id = "r1";
    rec.m_addr = "\"anar@lxg0527.gsi.de\"";
    rec.m_params = "-p24";
    rec.m_nWorkers = 4;
    BOOST_REQUIRE( recs[0] == rec );
    rec.m_id = "r2";
    rec.m_addr = "anar@lxi001.gsi.de";
    rec.m_params = "";
    rec.m_nWorkers = 2;
    BOOST_REQUIRE( recs[1] == rec );
    rec.m_id = "125";
    rec.m_addr = "anar@lxg0055.gsi.de";
    rec.m_params = "-p22";
    rec.m_nWorkers = 8;
    BOOST_REQUIRE( recs[2] == rec );
}
BOOST_AUTO_TEST_CASE( test_readconfig_bad )
{
    CConfig config;
    
    stringstream ss;
    ss << "r1, \\\"anar@lxg0527.gsi.de\\\", -p24, 4\n"
    << "r2, anar@lxi001.gsi.de,2\n"
    << "125, anar@lxg0055.gsi.de, -p22, 8\n";
    
    BOOST_REQUIRE_THROW(config.readFrom( ss ), runtime_error );
}

BOOST_AUTO_TEST_SUITE_END();
