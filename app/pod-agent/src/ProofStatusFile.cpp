/************************************************************************/
/**
 * @file ProofStatusFile.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-01-05
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010-2011 GSI, Scientific Computing devision. All rights reserved.
*************************************************************************/
#include "ProofStatusFile.h"
// STD
#include <fstream>
#include <stdexcept>
// BOOST
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
// MiscCommon
#include "def.h"
#include "MiscUtils.h"
#include "CustomIterator.h"
//=============================================================================
namespace fs = boost::filesystem;
using namespace PROOFAgent;
using namespace std;
using namespace MiscCommon;
//=============================================================================
template <class _T>
struct SFind
{
        SFind( const _T &_sign ): m_sign( _sign )
        {}
        bool operator()( const _T &_Val ) const
        {
            return ( _Val.find( m_sign ) != _Val.npos );
        }
    private:
        _T m_sign;
};
//=============================================================================
CProofStatusFile::CProofStatusFile():
    m_xpdPort( 0 ),
    m_xpdPid( 0 )
{
}
//=============================================================================
CProofStatusFile::~CProofStatusFile()
{
}
//=============================================================================
/// This function reads xpd.cfg and extracts adminpath paths for servers and workers.
/// Paths are parsed from a special comment block in PoD's xpd.cfg
/// The path is excluded if it doens't exist at the moment when the search is done.
bool CProofStatusFile::readAdminPath( const string &_xpdCFGFileName,
                                      EAdminPathType _type )
{
    //TODO: keeping fs::native is important for boost version earlier 1.34
    if( !fs::exists( fs::path( _xpdCFGFileName, fs::native ) ) )
        return false;

    // Read the content of the xpd.cf
    ifstream f( _xpdCFGFileName.c_str() );
    if( !f.is_open() )
        return false;

    StringVector_t vec;
    copy( custom_istream_iterator<string>( f ),
          custom_istream_iterator<string>(),
          back_inserter( vec ) );

    // Reading only comment blocks of proof.conf
    const string m( mark[static_cast<int>( _type )] );
    StringVector_t::iterator iter = find_if( vec.begin(), vec.end(),
                                             SFind<string>( m ) );
    if( iter == vec.end() )
        return false;

    iter->erase( iter->find( m ), m.size() );
    string p( *iter );
    trim<string>( &p, ' ' );

    // NOTE: "PoDServer" - is added to the admin path by xproofd.
    // we therefore also have to use it
    // This is only valid for PoD servers
    if( adminp_server == _type )
        p += "/PoDServer";

    //TODO: keeping fs::native is important for boost version earlier 1.34
    fs::path admin_path( p, fs::native );

    if( fs::exists( admin_path ) )
        swap( m_adminPath, admin_path );


    // find a xpd port
    const string xpd_str( "xpd.port" );
    iter = find_if( vec.begin(), vec.end(), SFind<string>( xpd_str ) );
    if( iter == vec.end() )
        return false;

    iter->erase( iter->find( xpd_str ), xpd_str.size() );
    string port( *iter );
    trim<string>( &port, ' ' );
    stringstream ss;
    string::const_iterator itrs = port.begin();
    string::const_iterator itrs_end = port.end();
    for( ; itrs != itrs_end; ++itrs )
    {
        if( !isdigit( *itrs ) )
            break;

        ss << *itrs;
    }
    ss >> m_xpdPort;

    // check for xpd pid
// Every time new xproofd is started it creates <adminpath>/.xproofd.port directory.
// In this directory an xrd pid file is located.
// There is one problem with the file, is that even when xrootd/xproofd is off already,
// the file will be there in anyway. This complicates the algorithm of detecting of xproofd.
    stringstream xpd_pid_file;
    xpd_pid_file
            << m_adminPath.string()
            << "/.xproofd." << m_xpdPort
            << "/xrootd.pid";

    ifstream f_pid( xpd_pid_file.str().c_str() );
    if( f_pid.is_open() )
        f_pid >> m_xpdPid;

    return true;
}
//=============================================================================
void CProofStatusFile::enumStatusFiles()
{
    // Cleaning
    m_files.clear();
    m_status.clear();

    if( m_adminPath.empty() )
        throw runtime_error( "Can't enumerate proof status files. No admin path is specified." );

    stringstream ss;
    ss << m_adminPath.string() << "/" << ".xproofd." << m_xpdPort << "/activesessions";
    //TODO: keeping fs::native is important for boost version earlier 1.34
    fs::path fullpath( ss.str(), fs::native );

    if( !fs::exists( fullpath ) )
    {
        stringstream ss;
        ss << "Can't enumerate proof status files. Admin path ["
           << fullpath.string()
           << "] doesn't exists.";
        throw runtime_error( ss.str() );
    }

    fs::directory_iterator end_itr; // default construction yields past-the-end
    for( fs::directory_iterator itr( fullpath ); itr != end_itr; ++itr )
    {
        if( fs::is_directory( *itr ) )
        {
            continue;
        }
        else if( fs::extension( itr->leaf() ) == ".status" )
        {
            m_files.push_back( *itr );

            // read a proof status from the file
            ifstream f( itr->string().c_str() );
            if( !f.is_open() )
            {
                // TODO: Think, whether we need to throw here
                stringstream ss;
                ss << "Can't open proof status file ["
                   << itr->string().c_str()
                   << "].";
                throw runtime_error( ss.str() );
            }
            int status( 0 );
            f >> status;
            m_status.push_back( static_cast<EProofStatus>( status ) );
        }
    }
}
