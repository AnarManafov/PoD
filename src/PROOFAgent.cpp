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
// STD
#include <stdexcept>
// PROOFAgent
#include "version.h"
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
    m_Data = _data->m_podOptions;
    m_Mode = _data->m_agentMode;
    initLogEngine();

    //InfoLog( MiscCommon::erOK, "PROOFAgent general configuration:" ) << m_Data;

    // Spawning new Agent in requested mode
    m_Agent.SetMode( m_Mode, _data );
}
//=============================================================================
void CPROOFAgent::Start() throw( exception )
{
    m_Agent.Start( m_Data.m_PROOFCfg );
}
//=============================================================================
void CPROOFAgent::initLogEngine()
{
    // Correcting configuration values
    // resolving user's home dir from (~/ or $HOME, if present)
    MiscCommon::smart_path( &m_Data.m_workDir );
    // We need to be sure that there is "/" always at the end of the path
    MiscCommon::smart_append<string>( &m_Data.m_workDir, '/' );

    MiscCommon::smart_path( &m_Data.m_logFileDir );
    MiscCommon::smart_append<string>( &m_Data.m_logFileDir, '/' );

    MiscCommon::smart_path( &m_Data.m_PROOFCfg );

    // Initializing log engine
    // log file name: proofagent.<instance_name>.pid
    std::stringstream logfile_name;
    logfile_name
    << m_Data.m_logFileDir
    << "proofagent."
    << ( m_Mode? "server" : "client" )
    << ".log";

    CLogSingleton::Instance().Init( logfile_name.str(), m_Data.m_logFileOverwrite );
// TODO:take VERSION from the build automatically
    InfoLog( erOK, PROJECT_NAME + string( " v.") + PROJECT_VERSION_STRING );

    // Timeout Guard
    if ( 0 != m_Data.m_agentTimeout )
        CTimeoutGuard::Instance().Init( getpid(), m_Data.m_agentTimeout );
}
//=============================================================================
void CPROOFAgent::ExecuteLastCmd()
{
    if ( !m_Data.m_lastExecCmd.empty() )
    {
        InfoLog( erOK, "executing last command: " + m_Data.m_lastExecCmd );
        if ( -1 == ::system( m_Data.m_lastExecCmd.c_str() ) )
            FaultLog( erError, "Can't execute last command: " + m_Data.m_lastExecCmd );
    }
}
