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
CPROOFAgent::CPROOFAgent()
{
}
//=============================================================================
CPROOFAgent::~CPROOFAgent()
{
    ExecuteLastCmd();
}
//=============================================================================
void CPROOFAgent::setConfiguration( const SOptions_t *_data )
{
    SAgentData_t tmp = _data->m_GeneralData;
    swap( m_Data, tmp );

    initLogEngine();

    InfoLog( MiscCommon::erOK, "PROOFAgent general configuration:" ) << m_Data;

    m_Data.m_AgentMode = ( m_Data.m_isServerMode ) ? Server : Client;
    // Spawning new Agent in requested mode
    m_Agent.SetMode( m_Data.m_AgentMode, _data );
}
//=============================================================================
void CPROOFAgent::Start() throw( exception )
{
    m_Agent.Start( m_Data.m_sPROOFCfg );
}
//=============================================================================
//void CPROOFAgent::loadCfg( const std::string &_fileName )
//{
// m_cfgFileName = _fileName;
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
