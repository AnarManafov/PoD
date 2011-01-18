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
    if( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );

    m_version = _data[0];
    m_version += ( _data[1] << 8 );
}
//=============================================================================
void SVersionCmd::_convertToData( MiscCommon::BYTEVector_t *_data ) const
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
    for( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_username.push_back( c );
    }

    for( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_host.push_back( c );
    }

    for( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_version.push_back( c );
    }

    for( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if( '\0' == c )
        {
            ++iter;
            ++idx;
            break;
        }
        m_PoDPath.push_back( c );
    }

    if( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );

    m_proofPort = _data[idx++];
    m_proofPort += ( _data[idx] << 8 );
}
//=============================================================================
void SHostInfoCmd::_convertToData( MiscCommon::BYTEVector_t *_data ) const
{
    std::copy( m_username.begin(), m_username.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );
    std::copy( m_host.begin(), m_host.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );
    std::copy( m_version.begin(), m_version.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );
    std::copy( m_PoDPath.begin(), m_PoDPath.end(), std::back_inserter( *_data ) );
    _data->push_back( '\0' );

    _data->push_back( m_proofPort & 0xFF );
    _data->push_back( m_proofPort >> 8 );
}
//=============================================================================
//=============================================================================
//=============================================================================
void SIdCmd::normalizeToLocal()
{
    m_id = inet::_normalizeRead32( m_id );
}
//=============================================================================
void SIdCmd::normalizeToRemote()
{
    m_id = inet::_normalizeWrite32( m_id );
}
//=============================================================================
void SIdCmd::_convertFromData( const MiscCommon::BYTEVector_t &_data )
{
    if( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );

    m_id = _data[0];
    m_id += ( _data[1] << 8 );
    m_id += ( _data[2] << 16 );
    m_id += ( _data[3] << 24 );
}
//=============================================================================
void SIdCmd::_convertToData( MiscCommon::BYTEVector_t *_data ) const
{
    _data->push_back( m_id & 0xFF );
    _data->push_back(( m_id >> 8 ) & 0xFF );
    _data->push_back(( m_id >> 16 ) & 0xFF );
    _data->push_back(( m_id >> 24 ) & 0xFF );
}
