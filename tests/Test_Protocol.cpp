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
// BOOST: tests
// Defines test_main function to link with actual unit test code.
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN    // Boost 1.33
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
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

    BYTEVector_t msg( createMsg( static_cast<uint16_t>(cmdVERSION), data ) );

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
BOOST_AUTO_TEST_CASE( test_create_checkMsg_cmdVERSION_badSize )
{
    BYTEVector_t data_return;
    BYTEVector_t msg;

    BOOST_CHECK_THROW( parseMsg( &data_return, msg ), runtime_error );

}
BOOST_AUTO_TEST_SUITE_END();
