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
//=============================================================================
#define PATH_CHECK( a, b ) check( a, b, __LINE__ )
void check( const fs::path & source,
            const std::string & expected, int line )
{
    if( source.string() == expected ) return;

    std::cout << '(' << line << ") source.string(): \"" << source.string()
              << "\" != expected: \"" << expected
              << "\"" << std::endl;
}
//=============================================================================
BOOST_AUTO_TEST_SUITE( pod_agent_ProtocolCommands );
//=============================================================================
BOOST_AUTO_TEST_CASE( test_getAdminPath_server )
{
    CProofStatusFile s;

    BOOST_REQUIRE( s.readAdminPath( "./xpd.cf", adminp_server ) );
    fs::path adminPath( s.getAdminPath() );
    BOOST_REQUIRE( !adminPath.empty() );
    BOOST_REQUIRE_EQUAL( adminPath.string(), INSTALL_PREFIX_TESTS"/PoDServer" );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_getAdminPath_worker )
{
    CProofStatusFile s;

    BOOST_REQUIRE( s.readAdminPath( "./xpd.cf", adminp_worker ) );
    fs::path adminPath( s.getAdminPath() );
    BOOST_REQUIRE( !adminPath.empty() );
    BOOST_REQUIRE_EQUAL( adminPath.string(), INSTALL_PREFIX_TESTS );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_enumStatusFiles_server )
{
    CProofStatusFile s;

    s.readAdminPath( "./xpd.cf", adminp_server );
    s.enumStatusFiles();
    PathVector_t files( s.getFiles() );
    BOOST_CHECK( !files.empty() );

    cout << "*SERVER* found files: " << endl;
    // TODO: use the following when switched to boost 1.34
    //copy(files.begin(), files.end(),
    //      ostream_iterator<fs::path>(cout, "\n"));
    PathVector_t::const_iterator iter = files.begin();
    PathVector_t::const_iterator iter_end = files.end();
    for( ; iter != iter_end; ++iter )
        cout << iter->string() << "\n";


    // checking the status
    ProofStatusContainer_t status( s.getStatus() );
    BOOST_CHECK( !status.empty() );
    cout << "found status: " << endl;
    copy( status.begin(), status.end(),
          ostream_iterator<EProofStatus>( cout, "\n" ) );
}
//=============================================================================
BOOST_AUTO_TEST_CASE( test_enumStatusFiles_worker )
{
    CProofStatusFile s;

    s.readAdminPath( "./xpd.cf", adminp_worker );
    s.enumStatusFiles();
    PathVector_t files( s.getFiles() );
    BOOST_CHECK( !files.empty() );

    cout << "*WORKER* found files: " << endl;
    // TODO: use the following when switched to boost 1.34
    //copy(files.begin(), files.end(),
    //      ostream_iterator<fs::path>(cout, "\n"));
    PathVector_t::const_iterator iter = files.begin();
    PathVector_t::const_iterator iter_end = files.end();
    for( ; iter != iter_end; ++iter )
        cout << iter->string() << "\n";


    // checking the status
    ProofStatusContainer_t status( s.getStatus() );
    BOOST_CHECK( !status.empty() );
    cout << "found status: " << endl;
    copy( status.begin(), status.end(),
          ostream_iterator<EProofStatus>( cout, "\n" ) );
}
//=============================================================================
BOOST_AUTO_TEST_SUITE_END();
