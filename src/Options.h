/************************************************************************/
/**
 * @file Options.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-05-27
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef OPTIONS_H
#define OPTIONS_H

#include "PARes.h"
#include <iostream>

namespace PROOFAgent
{

    /**
     *
     * @brief Agent's data structure.
     *
     */
    typedef struct SAgentData
    {
        SAgentData() :
                m_isServerMode( true ),
                m_bLogFileOverwrite( false ),
                m_nTimeout( 0 ),
                m_logLevel( 0 ),
                m_sPROOFCfg( "proof.conf" ),
                m_sWorkDir( "/tmp/" )
        {}
        bool m_isServerMode;    //!< Specify in which operation mode PA is started, Server of Client (default: true)
        std::string m_sLogFileDir;      //!< The log filename.
        bool m_bLogFileOverwrite;       //!< Overwrite log file each session.
        EAgentMode_t m_AgentMode;       //!< A mode of PROOFAgent, defined by ::EAgentMode_t.
        /**
         *
         * @brief It is a number of seconds, represents the time PROOFAgent instance is allowed to work.
         * @brief An internal timeout guard of the PROOFAgent will not allow PROOF agent to work longer, than
         * @brief it is instructed by this value and PROOFAgent will be forced to be killed. Default is 0 - no timeout.
         *
         */
        size_t m_nTimeout;
        size_t m_logLevel;
        std::string m_sLastExecCmd;     //!< PROOFAgent will execute this command at the end of the session.
        std::string m_sPROOFCfg;        //!< A location of the proof configuration file.
        std::string m_sWorkDir;         //!< Working folder. (default: /tmp/)
    }
    SAgentData_t;
//=============================================================================
    inline std::ostream &operator <<( std::ostream &_stream, const SAgentData_t &_data )
    {
        _stream
        << "Server mode: " << ( _data.m_isServerMode ? "yes" : "no" ) << "\n"
        << "Working directory: " << _data.m_sWorkDir << "\n"
        << "Log file directory: " << _data.m_sLogFileDir << "\n"
        << "Log file overwrite : " << ( _data.m_bLogFileOverwrite ? "yes" : "no" )
        << std::endl;
        return _stream;
    }
//=============================================================================
    /**
     *
     * @brief Agent's data structure (for server mode).
     *
     */
    typedef struct SAgentServerData
    {
        SAgentServerData() :
                m_nPort( 22222 ),
                m_nLocalClientPortMin( 20000 ),
                m_nLocalClientPortMax( 25000 )
        {}
        unsigned short m_nPort;
        unsigned short m_nLocalClientPortMin;
        unsigned short m_nLocalClientPortMax;
    }
    AgentServerData_t;
    inline std::ostream &operator <<( std::ostream &_stream, const AgentServerData_t &_data )
    {
        _stream
        << "Listen on Port: " << _data.m_nPort << "\n"
        << "a Local Clients Ports: " << _data.m_nLocalClientPortMin << "-" << _data.m_nLocalClientPortMax << std::endl;
        return _stream;
    }
//=============================================================================
    /**
     *
     * @brief Agent's data structure (for client mode).
     *
     */
    typedef struct SAgentClientData
    {
        SAgentClientData() :
                m_nServerPort( 22222 ),
                m_nLocalClientPort( 1093 ),
                m_shutdownIfIdleForSec(1800)
        {}
        unsigned short m_nServerPort;       //!< PROOFAgent's server port
        std::string m_strServerHost;        //!< PROOFAgent's server host
        unsigned short m_nLocalClientPort;  //!< PROOF's local port (on worker nodes)
        int m_shutdownIfIdleForSec;			//!< Shut down a worker if its idle time is higher this value. If value is 0 then the feature is off.
    }
    AgentClientData_t;

    //=============================================================================
    inline std::ostream &operator <<( std::ostream &_stream, const AgentClientData_t &_data )
    {
        _stream
        << "server info: [" << _data.m_strServerHost << ":" << _data.m_nServerPort << "];"
        << "\n"
        << "local listen port: " << _data.m_nLocalClientPort
        << "\n"
        << "Shut down if idle for: " <<  _data.m_shutdownIfIdleForSec
        << std::endl;
        return _stream;
    }

    /**
     *
     * @brief PROOFAgent's container of options
     *
     */
    typedef struct SOptions
    {
        typedef enum ECommand { Start, Stop, Status } ECommand_t;

        SOptions():                        // Default options' values
                m_Command( Start ),
                m_sPidfileDir( "/tmp/" ),
                m_bDaemonize( false ),
                m_bValidate( false )
        {}

        std::string m_sConfigFile;
        std::string m_sInstanceName;
        ECommand_t m_Command;
        std::string m_sPidfileDir;
        bool m_bDaemonize;
        bool m_bValidate;

        SAgentData m_GeneralData;
        SAgentServerData m_serverData;
        SAgentClientData m_clientData;
    } SOptions_t;
}

#endif
