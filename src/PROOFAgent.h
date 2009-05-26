/************************************************************************/
/**
 * @file PROOFAgent.h
 * @brief Header of the general PROOFAgent manager
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENT_H
#define PROOFAGENT_H

// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// PROOFAgent
#include "Agent.h"
#include "TimeoutGuard.h"

//=============================================================================
/**
 *
 * @brief A general name space for PROOFAgent application
 *
 */
namespace PROOFAgent
{
//=============================================================================
/**
 *
 * @brief Agent's data structure.
 *
 */
typedef struct SAgentData
{
    SAgentData() :
            m_isServerMode(true),
            m_bLogFileOverwrite( false ),
            m_nTimeout( 0 ),
            m_sWorkDir( "/tmp/" )
    {}
    bool m_isServerMode; 			//!< Specify in which operation mode PA is started, Server of Client (default: true)
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
    std::string m_sLastExecCmd;     //!< PROOFAgent will execute this command at the end of the session.
    std::string m_sPROOFCfg;        //!< A location of the proof configuration file.
    std::string m_sWorkDir;         //!< Working folder. (default: /tmp/)
}
SAgentData_t;
//=============================================================================
/**
 *
 * @brief The PROOFAgent manager
 *
 */
class CPROOFAgent:
            public MiscCommon::CLogImp<CPROOFAgent>
{
    friend class boost::serialization::access;
public:
    CPROOFAgent();
    virtual ~CPROOFAgent();

    REGISTER_LOG_MODULE( "PROOFAgent" )

public:
	void loadCfg( const std::string &_fileName );
    void Start() throw( std::exception );

private:
    template<class Archive>
    void save( Archive & _ar, const unsigned int /*_version*/ ) const
    {
        _ar
        & BOOST_SERIALIZATION_NVP( m_Data.m_isServerMode )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sWorkDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLogFileDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_bLogFileOverwrite )
        & BOOST_SERIALIZATION_NVP( m_Data.m_nTimeout )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLastExecCmd )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sPROOFCfg )
        & BOOST_SERIALIZATION_NVP( m_Agent );
    }
    template<class Archive>
    void load( Archive & _ar, const unsigned int /*_version*/ )
    {
        _ar
        & BOOST_SERIALIZATION_NVP( m_Data.m_isServerMode )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sWorkDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLogFileDir )
        & BOOST_SERIALIZATION_NVP( m_Data.m_bLogFileOverwrite )
        & BOOST_SERIALIZATION_NVP( m_Data.m_nTimeout )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sLastExecCmd )
        & BOOST_SERIALIZATION_NVP( m_Data.m_sPROOFCfg );

        m_Data.m_AgentMode = ( m_Data.m_isServerMode ) ? Server : Client;
        // Spawning new Agent in requested mode
        m_Agent.SetMode( m_Data.m_AgentMode );

        _ar & BOOST_SERIALIZATION_NVP( m_Agent );

        postLoad();
     }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    void ExecuteLastCmd();
    void postLoad();

private:
    SAgentData_t m_Data;
    CAgent m_Agent;
    std::string m_cfgFileName;
};

};
BOOST_CLASS_VERSION( PROOFAgent::CPROOFAgent, 1 )

#endif

