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

        Copyright (c) 2009-2011 GSI, Scientific Computing devision. All rights reserved.
*************************************************************************/
#include "ProtocolCommands.h"
// STD
#include <stdint.h>
#include <stdexcept>

// MiscCommon
#include "INet.h"
//=============================================================================
using namespace std;
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
    m_xpdPort = inet::_normalizeRead16( m_xpdPort );
    m_xpdPid = inet::_normalizeRead32( m_xpdPid );
    m_agentPort = inet::_normalizeRead16( m_agentPort );
    m_agentPid = inet::_normalizeRead32( m_agentPid );
}
//=============================================================================
void SHostInfoCmd::normalizeToRemote()
{
    m_xpdPort = inet::_normalizeWrite16( m_xpdPort );
    m_xpdPid = inet::_normalizeWrite32( m_xpdPid );
    m_agentPort = inet::_normalizeWrite16( m_agentPort );
    m_agentPid = inet::_normalizeWrite32( m_agentPid );
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

    m_xpdPort = _data[idx++];
    m_xpdPort += ( _data[idx] << 8 );

    ++idx;
    m_xpdPid = _data[idx++];
    m_xpdPid += ( _data[idx++] << 8 );
    m_xpdPid += ( _data[idx++] << 16 );
    m_xpdPid += ( _data[idx] << 24 );

    ++idx;
    m_agentPort = _data[idx++];
    m_agentPort += ( _data[idx] << 8 );

    ++idx;
    m_agentPid = _data[idx++];
    m_agentPid += ( _data[idx++] << 8 );
    m_agentPid += ( _data[idx++] << 16 );
    m_agentPid += ( _data[idx] << 24 );
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

    _data->push_back( m_xpdPort & 0xFF );
    _data->push_back( m_xpdPort >> 8 );

    _data->push_back( m_xpdPid & 0xFF );
    _data->push_back(( m_xpdPid >> 8 ) & 0xFF );
    _data->push_back(( m_xpdPid >> 16 ) & 0xFF );
    _data->push_back(( m_xpdPid >> 24 ) & 0xFF );

    _data->push_back( m_agentPort & 0xFF );
    _data->push_back( m_agentPort >> 8 );

    _data->push_back( m_agentPid & 0xFF );
    _data->push_back(( m_agentPid >> 8 ) & 0xFF );
    _data->push_back(( m_agentPid >> 16 ) & 0xFF );
    _data->push_back(( m_agentPid >> 24 ) & 0xFF );
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
//=============================================================================
//=============================================================================
//=============================================================================
void SWnListCmd::normalizeToLocal()
{
}
//=============================================================================
void SWnListCmd::normalizeToRemote()
{
}
//=============================================================================
void SWnListCmd::_convertFromData( const MiscCommon::BYTEVector_t &_data )
{
    m_container.clear();

    size_t idx( 0 );
    MiscCommon::BYTEVector_t::const_iterator iter = _data.begin();
    MiscCommon::BYTEVector_t::const_iterator iter_end = _data.end();
    string tmp_str;
    for( ; iter != iter_end; ++iter, ++idx )
    {
        char c( *iter );
        if( '\0' == c )
        {
            m_container.push_back( tmp_str );
            tmp_str.clear();
            continue;
        }
        tmp_str.push_back( c );
    }

    if( _data.size() < size() )
        throw std::runtime_error( "Protocol message data is too short" );
}
//=============================================================================
void SWnListCmd::_convertToData( MiscCommon::BYTEVector_t *_data ) const
{
    MiscCommon::StringVector_t::const_iterator iter = m_container.begin();
    MiscCommon::StringVector_t::const_iterator iter_end = m_container.end();
    for( ; iter != iter_end; ++iter )
    {
        std::copy( iter->begin(), iter->end(), std::back_inserter( *_data ) );
        _data->push_back( '\0' );
    }
}
