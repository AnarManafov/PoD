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

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "ProofStatusFile.h"
// STD
#include <fstream>
// BOOST
//#include <boost/filesystem/operations.hpp>
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
const char * const g_adminPath = "./.xproofd.22222/activesessions/";
//=============================================================================

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
CProofStatusFile::CProofStatusFile()
{
}
//=============================================================================
CProofStatusFile::~CProofStatusFile()
{
}
//=============================================================================
/// This function reads xpd.cfg and extracts adminpath paths for servers and workers.
/// Paths are parsed from a special comment block in PoD's xpd.cfg
/// A path is excluded if it doens't exist at the moment when the search is done.
bool CProofStatusFile::readAdminPath( const string &_xpdCFGFileName,
                                      EAdminPathType _type )
{
    if ( !fs::exists( fs::path( _xpdCFGFileName ) ) )
        return false;

    // Read the content of the xpd.cf
    ifstream f( _xpdCFGFileName.c_str() );
    if ( !f.is_open() )
        return false;

    StringVector_t vec;
    copy( custom_istream_iterator<string>( f ),
          custom_istream_iterator<string>(),
          back_inserter( vec ) );

    // Reading only comment blocks of proof.conf
    string m( mark[static_cast<int>( _type )] );
    StringVector_t::iterator iter = find_if( vec.begin(), vec.end(),
                                             SFind<string>( m ) );
    if ( iter == vec.end() )
        return false;

    iter->erase( iter->find( m ), m.size() );
    string p( *iter );
    trim<string>( &p, ' ' );

    fs::path admin_path( p );

    if ( fs::exists( admin_path ) )
        m_adminPath.swap( admin_path );

    return true;
}
//=============================================================================
void CProofStatusFile::enumStatusFiles( uint16_t _xpdPort )
{
    if ( m_adminPath.empty() )
        throw runtime_error( "Can't enumerate proof status files. No admin path is specified." );

    stringstream ss;
    ss << m_adminPath.string() << "/" << ".xproofd." << _xpdPort << "/activesessions";
    fs::path fullpath( ss.str() );

    if ( !fs::exists( fullpath ) )
    {
        stringstream ss;
        ss << "Can't enumerate proof status files. Admin path ["
        << fullpath.string()
        << "] doesn't exists.";
        throw runtime_error( ss.str() );
    }

    fs::directory_iterator end_itr; // default construction yields past-the-end
    for ( fs::directory_iterator itr( fullpath ); itr != end_itr; ++itr )
    {
        if ( fs::is_directory( *itr ) )
        {
            continue;
        }
        else if ( fs::extension( itr->leaf() ) == ".status" )
        {
            m_files.push_back( *itr );
        }
    }
}
