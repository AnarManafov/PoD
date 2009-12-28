/************************************************************************/
/**
 * @file ProtocolCommands.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-12-22
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "ProtocolCommands.h"
// STD
#include <stdint.h>
#include <stdexcept>
// MiscCommon
#include "INet.h"
//=============================================================================
using namespace PROOFAgent;
namespace inet = MiscCommon::INet;

//=============================================================================
void SVersionCmd::normalizeToLocal()
{
    m_version = inet::_normalizeRead16( m_version );
}
//=============================================================================
void SVersionCmd::normalizeToRemote()
{
    m_version = inet::_normalizeWrite16( m_version );
}
//=============================================================================
void SVersionCmd::_convertFromData( const MiscCommon::BYTEVector_t &_data )
{
    if ( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );

    m_version = _data[0];
    m_version += ( _data[1] << 8 );
}
//=============================================================================
void SVersionCmd::_convertToData( MiscCommon::BYTEVector_t *_data )
{
    _data->push_back( m_version & 0xFF );
    _data->push_back( m_version >> 8 );
}
//=============================================================================
//=============================================================================
//=============================================================================
void SHostInfoCmd::normalizeToLocal()
{
    m_proofPort = inet::_normalizeRead16( m_proofPort );
}
//=============================================================================
void SHostInfoCmd::normalizeToRemote()
{
    m_proofPort = inet::_normalizeWrite16( m_proofPort );
}
//=============================================================================
void SHostInfoCmd::_convertFromData( const MiscCommon::BYTEVector_t &_data )
{
    size_t idx( 0 );
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
    for ( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if ( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_username.push_back( c );
    }

    for ( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if ( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_host.push_back( c );
    }

    if ( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );

    m_proofPort = _data[idx++];
    m_proofPort += ( _data[idx] << 8 );
}
//=============================================================================
void SHostInfoCmd::_convertToData( MiscCommon::BYTEVector_t *_data )
{
    std::copy( m_username.begin(), m_username.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );
    std::copy( m_host.begin(), m_host.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );

    _data->push_back( m_proofPort & 0xFF );
    _data->push_back( m_proofPort >> 8 );
}
