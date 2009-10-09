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
// PROOFAgent
#include "version.h"
#include "PROOFAgent.h"
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
}
//=============================================================================
void CPROOFAgent::setConfiguration( const SOptions_t &_data )
{
    m_Mode = _data.m_agentMode;
    m_Data = ( Server == m_Mode ) ?
             _data.m_podOptions.m_server.m_common :
             _data.m_podOptions.m_worker.m_common;
    initLogEngine();

    //InfoLog( MiscCommon::erOK, "PROOFAgent general configuration:" ) << m_Data;

    // Spawning new Agent in requested mode
    m_Agent.SetMode( m_Mode, _data );
}
//=============================================================================
void CPROOFAgent::Start() throw( exception )
{
    m_Agent.Start();
}
//=============================================================================
void CPROOFAgent::initLogEngine()
{
    // Initializing log engine
    // log file name: proofagent.<instance_name>.log
    std::stringstream logfile_name;
    logfile_name
    << m_Data.m_logFileDir
    << "proofagent."
    << (( Server == m_Mode ) ? "server" : "client" )
    << ".log";

    unsigned char logLevel( LOG_SEVERITY_FAULT | LOG_SEVERITY_CRITICAL_ERROR );
    switch ( m_Data.m_logLevel )
    {
        case 3:
            logLevel |= LOG_SEVERITY_DEBUG;
        case 2:
            logLevel |= LOG_SEVERITY_WARNING;
        case 1:
            logLevel |= LOG_SEVERITY_INFO;
    }

    CLogSingleton::Instance().Init( logfile_name.str(),
                                    m_Data.m_logFileOverwrite,
                                    logLevel );
// TODO:take VERSION from the build automatically
    InfoLog( erOK, PROJECT_NAME + string( " v." ) + PROJECT_VERSION_STRING );
}
