//
//  MessageParser.cpp
//  PoD
//
//  Created by Anar Manafov on 01.06.11.
//  Copyright 2011 GSI. All rights reserved.
//
//=============================================================================
#include "MessageParser.h"
//=============================================================================
using namespace pod_remote;
using namespace std;
using namespace MiscCommon;
//=============================================================================
LPCSTR g_message_OK = "<pod-remote:OK>";
//=============================================================================
// SMessageParserOK
//=============================================================================
bool SMessageParserOK::operator()( const string &_buf )
{
    return ( _buf.find( g_message_OK ) != string::npos );
}
//=============================================================================
// SMessageParserNumber
//=============================================================================
bool SMessageParserNumber::operator()( const string &_buf )
{
    size_t pos = _buf.find( g_message_OK );
    if( pos == string::npos )
        return false;

    string b( _buf );
    b.erase( pos );
    m_num = 0;
    stringstream ss;
    ss << b;
    ss >> m_num;

    return true;
}
//=============================================================================
const int SMessageParserNumber::get() const
{
    return m_num;
}
//=============================================================================
// SMessageParserString
//=============================================================================
#include <iostream>
bool SMessageParserString::operator()( const string &_buf )
{
    size_t pos = _buf.find( g_message_OK );
    if( pos == string::npos )
        return false;

    m_str = _buf;
    m_str.erase( pos );
    
    return true;
}
//=============================================================================
const string SMessageParserString::get() const
{
    return m_str;
}
//=============================================================================
