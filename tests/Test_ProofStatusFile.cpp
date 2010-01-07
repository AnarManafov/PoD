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
#include "version.h"
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
    CProofStatusFile s;

    BOOST_REQUIRE( s.readAdminPath( "./xpd.cf", adminp_server ) );
    fs::path adminPaths( s.getAdminPath() );
    BOOST_REQUIRE( !adminPaths.empty() );
    PATH_CHECK( adminPaths, INSTALL_PREFIX_TESTS );

    BOOST_REQUIRE( s.readAdminPath( "./xpd.cf", adminp_worker ) );
    adminPaths = s.getAdminPath();
    BOOST_REQUIRE( !adminPaths.empty() );
    PATH_CHECK( adminPaths, INSTALL_PREFIX_TESTS"/" );
}
//=============================================================================
//#include <iterator>
BOOST_AUTO_TEST_CASE( test_enumStatusFiles )
{
    CProofStatusFile s;

    s.readAdminPath( "./xpd.cf", adminp_server );
    s.enumStatusFiles(22222);
    PathVector_t files( s.getFiles() );
    BOOST_CHECK( !files.empty() );

    cout << "found files: " << endl;
    copy(files.begin(), files.end(),
    		ostream_iterator<fs::path>(cout, "\n"));

    // checking the status
    ProofStatusContainer_t status( s.getStatus() );
    BOOST_CHECK( !status.empty() );
    cout << "found status: " << endl;
       copy(status.begin(), status.end(),
       		ostream_iterator<EProofStatus>(cout, "\n"));
}
BOOST_AUTO_TEST_SUITE_END();
