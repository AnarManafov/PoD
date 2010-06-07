/*
 *  config.cpp
 *  pod-ssh
 *
 *  Created by Anar Manafov on 07.06.10.
 *  Copyright 2010 Anar Manafov <Anar.Manafov@gmail.com>. All rights reserved.
 *
 */
//=============================================================================
// std
#include <iostream>
// boost
#include <boost/tokenizer.hpp>
// MiscCommon
#include "CustomIterator.h"
#include "def.h"
// pod-ssh
#include "config.h"
//=============================================================================
using namespace MiscCommon;
using namespace std;
//=============================================================================
typedef boost::tokenizer<boost::escaped_list_separator<char> > Tok;
//=============================================================================
void CConfig::readFrom( istream &_stream )
{
    // get lines from the configuration
    StringVector_t lines;
    copy( custom_istream_iterator<string>( _stream ),
          custom_istream_iterator<string>(),
          back_inserter( lines ) );

    // pars the configuration using boost's tokenizer
    SConfigRecord rec;
    StringVector_t::const_iterator iter = lines.begin();
    StringVector_t::const_iterator iter_end = lines.end();
    for( size_t i = 0; iter != iter_end; ++iter, ++i )
    {
        Tok t( *iter );
        int res = rec.assignValues( t.begin(), t.end() );
        if( res )
        {
            stringstream ss;
            ss << "pod-ssh configuration: syntax error at line "
               << i + 1;
            throw runtime_error( ss.str() );
        }
        // save a configuration record to a container
        m_records.push_back( rec );
    }
}
//=============================================================================
configRecords_t CConfig::getRecords()
{
    return m_records;
}
