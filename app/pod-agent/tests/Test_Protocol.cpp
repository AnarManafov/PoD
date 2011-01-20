/************************************************************************/
/**
 * @file Test_Protocol.cpp
 * @brief Unit tests of Protocol
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN    // Boost 1.33
#define BOOST_TEST_MAIN

// FIX: silence a warning until BOOST fix it
// boost/test/floating_point_comparison.hpp:251:25: warning: unused variable 'check_is_close' [-Wunused-variable]
// boost/test/floating_point_comparison.hpp:273:25: warning: unused variable 'check_is_small' [-Wunused-variable]
// clang
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
#include <boost/test/auto_unit_test.hpp>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

using boost::unit_test::test_suite;
// pod-agent
#include "Protocol.h"
#include "ProtocolCommands.h"
//=============================================================================
using namespace std;
using namespace PROOFAgent;
using namespace MiscCommon;

BOOST_AUTO_TEST_SUITE( pod_agent_Protocol );
//=============================================================================
BOOST_AUTO_TEST_CASE( test_create_checkMsg_cmdVERSION )
{
    SVersionCmd a;
    a.m_version = 34;
    BYTEVector_t data;
    a.convertToData( &data );

    BYTEVector_t msg( createMsg( static_cast<uint16_t>( cmdVERSION ), data ) );

    BYTEVector_t data_return;
    SMessageHeader header( parseMsg( &data_return, msg ) );
    BOOST_CHECK( header.isValid() );
    BOOST_CHECK_EQUAL( header.m_cmd, cmdVERSION );
    BOOST_CHECK_EQUAL( header.m_len, a.size() );

    SVersionCmd b;
    b.convertFromData( data_return );
    BOOST_CHECK_EQUAL( a, b );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_create_checkMsg_cmdVERSION_badSize0 )
{
    BYTEVector_t data_return;
    BYTEVector_t msg;

    SMessageHeader header( parseMsg( &data_return, msg ) );
    BOOST_CHECK( !header.isValid() );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_create_checkMsg_cmdVERSION_badSize1 )
{
    BYTEVector_t data_return;
    BYTEVector_t msg( 50 );

    BOOST_CHECK_THROW( parseMsg( &data_return, msg ), runtime_error );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_create_checkMsg_cmdHOST_INFO )
{
    SHostInfoCmd a;
    a.m_username = "testuser";
    a.m_host = "test.host.de";
    a.m_xpdPort = 129;
    a.m_version = "5.05.05daf";
    a.m_PoDPath = "/opt/local/trara/sdf";

    BYTEVector_t data;
    a.convertToData( &data );

    BYTEVector_t msg( createMsg( static_cast<uint16_t>( cmdHOST_INFO ), data ) );

    BYTEVector_t data_return;
    SMessageHeader header( parseMsg( &data_return, msg ) );
    BOOST_CHECK( header.isValid() );
    BOOST_CHECK_EQUAL( header.m_cmd, cmdHOST_INFO );
    BOOST_CHECK_EQUAL( header.m_len, a.size() );

    SHostInfoCmd b;
    b.convertFromData( data_return );
    BOOST_CHECK_EQUAL( a, b );
}
BOOST_AUTO_TEST_SUITE_END();
