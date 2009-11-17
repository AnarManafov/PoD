/************************************************************************/
/**
 * @file LsfMng.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-12-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <stdexcept>
#include <cstring>
// Misc
#include "def.h"
// LSF plug-in
#include "LsfMng.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
//=============================================================================
const LPCTSTR g_szAppName = "PoD LSF plug-in";
//=============================================================================
CLsfMng::CLsfMng(): m_bInit( false )
{

}
//=============================================================================
CLsfMng::~CLsfMng()
{

}
//=============================================================================
void CLsfMng::init()
{
    // initialize LSBLIB  and  get  the  configuration environment
    // FIX: for some reason lsb_init requires char * instead of const char *. This needs to be investigated
    if ( lsb_init( const_cast<char*>( g_szAppName ) ) < 0 )
        throw runtime_error( "Can't initialize LSF." ); // TODO: get error description here (get it from LSF, lsberrno)

    m_submitRequest.clear();

    m_bInit = true;
}
//=============================================================================
void CLsfMng::addProperty( EJobProperty_t _type, const string &_val )
{
    m_submitRequest[_type] = _val;
}
//=============================================================================
lsf_jobid_t CLsfMng::jobSubmit( const std::string &_Cmd )
{
    if ( !m_bInit )
        return 0; //TODO: throw something here

    submit request;

    // set all defaults
    memset( &request, 0, sizeof( submit ) );
    for ( size_t i = 0; i < LSF_RLIM_NLIMITS; i++ )
        request.rLimits[i] = DEFAULT_RLIMIT;

    // TODO: implement this via STD algorithms (accumulate would fit I think)
    propertyDict_t::const_iterator iter = m_submitRequest.begin();
    propertyDict_t::const_iterator iter_end = m_submitRequest.end();
    for ( ; iter != iter_end; ++iter )
    {
        // TODO: investigate whether LSF really needs "char *"! Meantime removing const from our const strings.
        switch ( iter->first )
        {
            case JP_SUB_JOB_NAME:
                request.options |= SUB_JOB_NAME;
                request.jobName = const_cast<char*>( iter->second.c_str() );
                break;
            case JP_SUB_QUEUE:
                request.options |= SUB_QUEUE;
                request.queue = const_cast<char*>( iter->second.c_str() );
                break;
            case JP_SUB_HOST:
                request.options |= SUB_HOST;
                request.hostSpec = const_cast<char*>( iter->second.c_str() );
                break;
            case JP_SUB_IN_FILE:
                request.options |= SUB_IN_FILE;
                request.inFile = const_cast<char*>( iter->second.c_str() );
                break;
            case JP_SUB_OUT_FILE:
                request.options |= SUB_OUT_FILE;
                request.outFile = const_cast<char*>( iter->second.c_str() );
                break;
            case JP_SUB_ERR_FILE:
                request.options |= SUB_ERR_FILE;
                request.errFile = const_cast<char*>( iter->second.c_str() );
                break;
            default:
                return 0; //TODO: Assert here
        }
    }

    request.command = const_cast<char*>( _Cmd.c_str() );

    submitReply reply; // results of job submission
    // submit the job with specifications
    int jobId = lsb_submit( &request, &reply );

    if ( jobId < 0 )
    {
        // if job submission fails, lsb_submit returns -1
        switch ( lsberrno )
        {
                // and sets lsberrno to indicate the error
            case LSBE_QUEUE_USE:
            case LSBE_QUEUE_CLOSED:
            default:
                throw runtime_error( "job submission failed" ); // TODO: report a proper error here
        }
        return 0;
    }
    return LSB_JOBID( jobId, 0 );
}
//=============================================================================
CLsfMng::EJobStatus_t CLsfMng::jobStatus( lsf_jobid_t _jobID ) const
{
    if ( !m_bInit )
        return JS_JOB_STAT_UNKWN;

    // detailed job info
    jobInfoEnt *job;

    if ( lsb_openjobinfo( _jobID, NULL, NULL, NULL, NULL, ALL_JOB ) < 0 )
        return JS_JOB_STAT_UNKWN;

    // number of remaining jobs unread
    int more = 0;
    // get the job details
    job = lsb_readjobinfo( &more );
    if ( NULL == job )
    {
        lsb_closejobinfo();
        return JS_JOB_STAT_UNKWN;
    }

    if ( 0 != job->endTime )
        return JS_JOB_STAT_COMPLETED;

    EJobStatus_t status = static_cast<EJobStatus_t>( job->status );

    //when finished to display the job info, close the connection to the mbatchd
    lsb_closejobinfo();

    return status;
}
//=============================================================================
std::string CLsfMng::jobStatusString( lsf_jobid_t _jobID ) const
{
    switch ( jobStatus( _jobID ) )
    {
        case JS_JOB_STAT_PEND:
            return "pending";
        case JS_JOB_STAT_PSUSP:
            return "held";
        case JS_JOB_STAT_RUN:
            return "running";
        case JS_JOB_STAT_SSUSP:
            return "suspended by LSF";
        case JS_JOB_STAT_USUSP:
            return "suspended by user";
        case JS_JOB_STAT_EXIT:
            return "exited";
        case JS_JOB_STAT_DONE:
            return "completed";
        case JS_JOB_STAT_PDONE:
            return "done";
        case JS_JOB_STAT_PERROR:
            return "job process error";
        case JS_JOB_STAT_WAIT:
            return "waiting";
        case JS_JOB_STAT_COMPLETED:
            return "completed";
        default:
            return "unknown";
    }
}
//=============================================================================
std::string CLsfMng::jobStatusString( CLsfMng::EJobStatus_t _jobStatus ) const
{
    switch ( _jobStatus )
    {
        case JS_JOB_STAT_PEND:
            return "pending";
        case JS_JOB_STAT_PSUSP:
            return "held";
        case JS_JOB_STAT_RUN:
            return "running";
        case JS_JOB_STAT_SSUSP:
            return "suspended by LSF";
        case JS_JOB_STAT_USUSP:
            return "suspended by user";
        case JS_JOB_STAT_EXIT:
            return "exited";
        case JS_JOB_STAT_DONE:
            return "completed";
        case JS_JOB_STAT_PDONE:
            return "done";
        case JS_JOB_STAT_PERROR:
            return "job process error";
        case JS_JOB_STAT_WAIT:
            return "waiting";
        case JS_JOB_STAT_COMPLETED:
            return "completed";
        default:
            return "unknown";
    }
}
//=============================================================================
int CLsfMng::getNumberOfChildren( lsf_jobid_t _jobID ) const
{
    if ( !m_bInit )
        return 0;

    // detailed job info
    jobInfoEnt *job;

    //gets the total number of pending job. Exits if failure */
    if ( lsb_openjobinfo( _jobID, NULL, NULL, NULL, NULL, ALL_JOB | JGRP_ARRAY_INFO ) < 0 )
        return 0;

    // number of remaining jobs unread
    int more = 0;
    // get the job details
    job = lsb_readjobinfo( &more );
    if ( job == NULL )
    {
        lsb_closejobinfo();
        return 0;
    }

    int retNumberOfJobsInArray = 0;
    // check whether it is an array job
    if ( JGRP_NODE_ARRAY == job->jType )
    {
        retNumberOfJobsInArray = job->counter[JGRP_COUNT_NJOBS];
    }

    //when finished to display the job info, close the connection to the mbatchd
    lsb_closejobinfo();

    return retNumberOfJobsInArray;
}
//=============================================================================
void CLsfMng::getChildren( lsf_jobid_t _jobID, IDContainer_t *_container ) const
{
    if ( !_container )
        return;

    int children_count = getNumberOfChildren( _jobID );
    if ( 0 >= children_count )
        return;

    for ( int i = 0; i < children_count; ++i )
    {
        _container->push_back( LSB_JOBID( _jobID, i ) );
    }
}
//=============================================================================
void CLsfMng::getQueues( LSFQueueInfoMap_t *_retVal ) const
{
    if ( !_retVal )
        return;

    struct queueInfoEnt *qInfo;
    char *queues;
    int numQueues = 0;
    char *host = NULL;
    char *user = NULL;
    int options = 0;

    // get queue information about the specified queue
    qInfo = lsb_queueinfo( &queues, &numQueues, host, user, options );
    if ( NULL == qInfo )
    {
        //lsb_perror("simbqueues: lsb_queueinfo() failed");
        return; // TODO: need exception here
    }
    for ( int i = 0; i < numQueues; ++i )
    {
        SLSFQueueInfo_t info;
        info.m_userJobLimit = qInfo[i].userJobLimit;
        _retVal->insert( LSFQueueInfoMap_t::value_type( qInfo[i].queue, info ) );
    }
}
