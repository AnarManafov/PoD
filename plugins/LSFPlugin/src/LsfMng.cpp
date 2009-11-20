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
#include <iostream>
// Misc
#include "def.h"
#include "SysHelper.h"
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

    get_cuser_name( &m_user );

    m_bInit = true;

    cout << "init" << endl;
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
        char *msg = lsb_sysmsg();
        stringstream ss;
        ss << "Job submission failed. LSF error: ";
        if ( NULL != msg )
            ss << msg;
        else
            ss << lsberrno;
        ss << "\nPlease, try again.";
        throw runtime_error( ss.str() );
    }
    return jobId;//LSB_JOBID( jobId, 0 );
}
//=============================================================================
string CLsfMng::jobStatusString( int _status )
{
    switch ( _status )
    {
        case JOB_STAT_NULL:
            return string( "null" );
        case JOB_STAT_PEND:
            return string( "pending" );
        case JOB_STAT_PSUSP:
            return string( "held" );
        case JOB_STAT_RUN:
            return string( "run" );
        case JOB_STAT_RUN|JOB_STAT_WAIT:
            return string( "waiting" );
        case JOB_STAT_SSUSP:
            return string( "suspended by LSF" );
        case JOB_STAT_USUSP:
            return string( "suspended by user" );
        case JOB_STAT_EXIT:
            //if ( jobInfo->reasons & EXIT_ZOMBIE )
            //    return string( "ZOMBI" );
            //else
            return string( "exit" );
        case JOB_STAT_DONE:
        case JOB_STAT_DONE|JOB_STAT_PDONE:
        case JOB_STAT_DONE|JOB_STAT_PERR:
        case JOB_STAT_DONE|JOB_STAT_WAIT:
            return string( "done" );

        case JOB_STAT_UNKWN:
            return string( "unknown" );
        default:
            return string( "can't get status" );
    }
}
//=============================================================================
int CLsfMng::getNumberOfChildren( lsf_jobid_t _jobID ) const
{
    if ( !m_bInit )
        return 0;

    // detailed job info
    jobInfoEnt *job;

    cout << "getNumberOfChildren" << endl;

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
        _container->push_back( LSB_JOBID( _jobID, i + 1 ) );
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
//=============================================================================
void CLsfMng::killJob( lsf_jobid_t _jobID ) const
{
    int res = lsb_signaljob( _jobID, SIGKILL );
    if ( res < 0 )
    {
        char *msg = lsb_sysmsg();
        stringstream ss;
        ss << "LSF message: ";
        if ( NULL != msg )
            ss << msg;
        else
        {
            ss << lsberrno;
        }
        throw runtime_error( ss.str() );
    }
}
//=============================================================================
size_t CLsfMng::getAllUnfinishedJobs( IDContainerOrdered_t *_container ) const
{
	size_t countJobs(0);
    if ( !_container )
        return countJobs;

    cout << "getAllUnfinishedJobs" << endl;


    // Retrieve all job ids of the current user, jobs which have not finished yet
    if ( lsb_openjobinfo( 0, NULL, const_cast<char*>( m_user.c_str() ), NULL, NULL, CUR_JOB ) > 0 )
    {
        jobInfoEnt *job;
        while (( job = lsb_readjobinfo( NULL ) ) != NULL )
        {
        	++countJobs;
            _container->insert( IDContainerOrdered_t::value_type( job->jobId, job->status ) );

            // TODO: for parent jobs just print a statistics information (X - pending; Y - run; ...)
            // when TODO is implemented, we can remove the following...
            // adding a parent of this jobs, so that we can track it in case when at least one of its children is running
            // if we don't do that, than parent id will be never in the list and will never get a status updated
            // We also shouldn't count this jobs in the return value.
            _container->insert( IDContainerOrdered_t::value_type( LSB_ARRAY_JOBID( job->jobId ), job->status ) );
        }
    }
    lsb_closejobinfo();
    return countJobs;
}
