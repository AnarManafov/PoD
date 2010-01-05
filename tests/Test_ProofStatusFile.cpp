/************************************************************************/
/**
 * @file Test_ProofStatusFile.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-01-05
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
// pod-agent
#include "ProofStatusFile.h"
// MiscCommon
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace PROOFAgent;
using boost::unit_test::test_suite;
using namespace MiscCommon;
namespace fs = boost::filesystem;

#define PATH_CHECK( a, b ) check( a, b, __LINE__ )
void check( const fs::path & source,
            const std::string & expected, int line )
{
    if ( source.string() == expected ) return;

    std::cout << '(' << line << ") source.string(): \"" << source.string()
              << "\" != expected: \"" << expected
              << "\"" << std::endl;
}

BOOST_AUTO_TEST_SUITE( pod_agent_ProtocolCommands );
//=============================================================================

//=============================================================================
BOOST_AUTO_TEST_CASE( test_getAdminPath )
{
    CTest_CProofStatusFile s;

    fs::path adminPaths;
    BOOST_CHECK( s.getAdminPath( "./xpd.cf", &adminPaths, adminp_server ) );
    BOOST_CHECK( !adminPaths.empty() );
    PATH_CHECK( adminPaths, "./" );

    BOOST_CHECK( s.getAdminPath( "./xpd.cf", &adminPaths, adminp_worker ) );
    BOOST_CHECK( !adminPaths.empty() );
    PATH_CHECK( adminPaths, "./" );
}
BOOST_AUTO_TEST_SUITE_END();
