/************************************************************************/
/**
 * @file PROOFAgent.cpp
 * @brief Implementation of the general PROOFAgent manager
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
// STD
#include <stdexcept>
// BOOST
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
// PROOFAgent
#include "PROOFAgent.h"
// MiscCommon
#include "MiscUtils.h"
#include "SysHelper.h"
#include "FindCfgFile.h"
//=============================================================================
#define MAX_PATH 1024
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
//=============================================================================
template<class T>
void _loadcfg( T &_s, string _FileName )
{
    smart_path( &_FileName );
    if ( _FileName.empty() || !is_file_exists( _FileName ) )
        throw exception();

    ifstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_iarchive ia( f );
    ia >> BOOST_SERIALIZATION_NVP( _s );
}
//=============================================================================
template<class T>
void _savecfg( const T &_s, string _FileName )
{
    smart_path( &_FileName );
    if ( _FileName.empty() )
        throw runtime_error("Can't update the configuration file. Illigal name of the file.");

    // make an archive
    ofstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_oarchive oa( f );
    oa << BOOST_SERIALIZATION_NVP( _s );
}
//=============================================================================
CPROOFAgent::CPROOFAgent()
{
}
//=============================================================================
CPROOFAgent::~CPROOFAgent()
{
//    try
//    {
//        // Saving class to the config file
//        _savecfg( *this, m_cfgFileName );
//    }
//    catch ( const exception &_e )
//    {
//        FaultLog( erError, _e.what() );
//    }
//    catch ( ... )
//    {
//        // TODO: log message
//    }

    ExecuteLastCmd();
}
//=============================================================================
void CPROOFAgent::setConfiguration( SAgentData _data )
{
	swap(m_Data, _data);
}
//=============================================================================
void CPROOFAgent::Start() throw( exception )
{
    m_Agent.Start( m_Data.m_sPROOFCfg );
}
//=============================================================================
//void CPROOFAgent::loadCfg( const std::string &_fileName )
//{
//	m_cfgFileName = _fileName;
//    try
//    {
//        // Loading class from the config file
//        _loadcfg( *this, _fileName );
//    }
//    catch ( ... )
//    {
//        cerr << "PROOFAgent error: "
//        << "Can't load configuration file "
//        << m_cfgFileName << endl;
//        //   exit(0); // TODO: revise this case
//    }
//}
//=============================================================================
void CPROOFAgent::initLogEngine()
{
    // Correcting configuration values
    // resolving user's home dir from (~/ or $HOME, if present)
    MiscCommon::smart_path( &m_Data.m_sWorkDir );
    // We need to be sure that there is "/" always at the end of the path
    MiscCommon::smart_append<string>( &m_Data.m_sWorkDir, '/' );

    MiscCommon::smart_path( &m_Data.m_sLogFileDir );
    MiscCommon::smart_append<string>( &m_Data.m_sLogFileDir, '/' );

    MiscCommon::smart_path( &m_Data.m_sPROOFCfg );

    // Initializing log engine
    // log file name: proofagent.<instance_name>.pid
    std::stringstream logfile_name;
    logfile_name
    << m_Data.m_sLogFileDir
    << "proofagent."
    << (( m_Data.m_isServerMode ) ? "server" : "client" )
    << ".log";

    CLogSingleton::Instance().Init( logfile_name.str(), m_Data.m_bLogFileOverwrite );
// TODO:take VERSION from the build automatically
    InfoLog( erOK, string( "proofagent v." ) + "2.0.0" );

    // Timeout Guard
    if ( 0 != m_Data.m_nTimeout )
        CTimeoutGuard::Instance().Init( getpid(), m_Data.m_nTimeout );
}
//=============================================================================
void CPROOFAgent::ExecuteLastCmd()
{
    if ( !m_Data.m_sLastExecCmd.empty() )
    {
        InfoLog( erOK, "executing last command: " + m_Data.m_sLastExecCmd );
        if ( -1 == ::system( m_Data.m_sLastExecCmd.c_str() ) )
            FaultLog( erError, "Can't execute last command: " + m_Data.m_sLastExecCmd );
    }
}
