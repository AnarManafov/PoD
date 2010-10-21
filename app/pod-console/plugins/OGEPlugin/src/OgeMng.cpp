/************************************************************************/
/**
 * @file OgeMng.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-10-13
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// OGE plug-in
#include "OgeMng.h"
// API
#include <sys/stat.h>
// OGE API
#include "drmaa.h"
// STD
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
// Qt
#include <QtGlobal>
// MiscCommon
#include "SysHelper.h"
#include "Process.h"

using namespace std;
using namespace oge_plug;
using namespace MiscCommon;
//=============================================================================
const LPCSTR g_OGEOptionFile = "$POD_LOCATION/etc/Job.oge.option";
//=============================================================================
ostream &SQueueInfo::print( ostream &_stream ) const
{
    _stream
            << "queue name: " << m_name
            << "; max jobs: " << m_maxJobs
            << "\n";

    return _stream;
}
struct SFindQueue
{
    bool operator()( const SQueueInfo &_info )
    {
        // "proof" is a default queue name
        return( _info.m_name >= "proof" );
    }
};
//=============================================================================
/**
 * @brief Initializes the drmaa library.
 *
 */
void COgeMng::initDRMAA() const
{
    int nRetries( 0 );
    int errnum = 1;
    char error[DRMAA_ERROR_STRING_BUFFER];

    while( errnum != 0 && nRetries < 3 )
    {
        errnum = drmaa_init( NULL, error, DRMAA_ERROR_STRING_BUFFER );
        nRetries++;
    }
    if( DRMAA_ERRNO_SUCCESS != errnum )
        throw runtime_error( error );
}
//=============================================================================
void COgeMng::exitDRMAA() const
{
    char error[DRMAA_ERROR_STRING_BUFFER];
    int errnum = drmaa_exit( error, DRMAA_ERROR_STRING_BUFFER );
    if( DRMAA_ERRNO_SUCCESS != errnum )
        throw runtime_error( error );
}
//=============================================================================
void COgeMng::setUserDefaults( const PoD::CPoDUserDefaults &_ud )
{
    try
    {
        m_server_logDir = _ud.getValueForKey( "server.logfile_dir" );
        smart_path( &m_server_logDir );
        smart_append( &m_server_logDir, '/' );

//        stringstream ss;
//        ss << _ud.getValueForKey( "oge_plugin.shared_home" );
//        ss >> m_oge_sharedHome;
    }
    catch( exception &e )
    {
    }
}
//=============================================================================
bool COgeMng::isValid( const jobID_t &_id )
{
    return !_id.empty();
}
//=============================================================================
string COgeMng::getCleanParentID( const jobID_t &_id ) const
{
    // JobID in DRMAA OGE: PARENTID.ARRAYINDEX
    // Clean ParentID is the PARENTID parent of these ids.
    jobID_t::size_type pos = _id.find( '.' );
    if( jobID_t::npos == pos )
        return _id;

    return _id.substr( 0, pos );
}
//=============================================================================
bool COgeMng::isParentID( const jobID_t &_parent )
{
    jobID_t::size_type pos = _parent.find( '.' );
    return ( jobID_t::npos == pos );
}
//=============================================================================
COgeMng::jobID_t COgeMng::generateArrayJobID( const jobID_t &_parent,
                                              size_t _idx )
{
    // to get an array ID we need to add ".index" to a parentID, to get
    jobID_t::size_type pos = _parent.find( '.' );
    if( jobID_t::npos == pos )
        return _parent;

    stringstream ss( _parent );
    ss << "." << _idx;
    return ss.str();
}
//=============================================================================
COgeMng::jobArray_t COgeMng::jobSubmit( const string &_script, const string &_queue,
                                        size_t _nJobs ) const
{
    jobArray_t ret;
    try
    {
        initDRMAA();

        char error[DRMAA_ERROR_STRING_BUFFER];
        int errnum = 0;
        drmaa_job_template_t *jt = NULL;

        // Create a job template
        errnum = drmaa_allocate_job_template( &jt, error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not create job template: " );
            msg += error;
            throw runtime_error( msg );
        }

        // set job's attributes
        errnum = drmaa_set_attribute( jt, DRMAA_REMOTE_COMMAND, _script.c_str(),
                                      error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set attribute " );
            msg += DRMAA_REMOTE_COMMAND;
            msg += " :";
            msg += error;
            throw runtime_error( msg );
        }

        string nativeSpecification( getDefaultNativeSpecification( _queue, _nJobs ) );
        errnum = drmaa_set_attribute( jt, DRMAA_NATIVE_SPECIFICATION, ( char * )nativeSpecification.c_str(),
                                      error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set attribute " );
            msg += DRMAA_NATIVE_SPECIFICATION;
            msg += " :";
            msg += error;
            throw runtime_error( msg );
        }

        errnum = drmaa_set_attribute( jt, DRMAA_V_ENV, ( char * )getEnvArray().c_str(),
                                      error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set environment " );
            msg += DRMAA_V_ENV;
            msg += " :";
            msg += error;
            throw runtime_error( msg );
        }

        // Submit the array job
        drmaa_job_ids_t *ids = NULL;
        errnum = drmaa_run_bulk_jobs( &ids, jt, jobArrayStartIdx(), _nJobs, 1, error,
                                      DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not submit job: " );
            msg += error;
            throw runtime_error( msg );
        }

        char jobid[DRMAA_JOBNAME_BUFFER];
        // reserve the first position
        ret.push_back( "0" );
        while( drmaa_get_next_job_id( ids, jobid, DRMAA_JOBNAME_BUFFER ) == DRMAA_ERRNO_SUCCESS )
        {
            ret.push_back( jobid );
        }

        if( ret.size() > 1 )
        {
            // push first the fake parrent id
            ret[0] = getCleanParentID( ret[1] );
        }
        else
        {
            // something is wrong, we therefore remove the fake parent reservation
            ret.clear();
        }

        drmaa_release_job_ids( ids );

        // creating a log dir for the job
        createJobsLogDir( jobid );
    }
    catch( ... )
    {
        exitDRMAA();
        throw;
    }
    exitDRMAA();

    return ret;
}
//=============================================================================
string COgeMng::getEnvArray() const
{
    string env;
    // set POD_UI_LOCATION on the worker nodes
    char *loc = getenv( "POD_LOCATION" );
    if( loc != NULL )
    {
        env += "POD_UI_LOCATION=";
        env += loc;
        env += ' ';
    }
    // set POD_UI_LOG_LOCATION variable on the worker nodes
    env += "POD_UI_LOG_LOCATION=";
    env += m_server_logDir;
    env += ' ';

//    // set POD_OGE_SHARED_HOME variable on the worker nodes
//    env += "POD_OGE_SHARED_HOME=";
//    env += m_oge_sharedHome ? "1" : "0";

    // export all env. variables of the process to jobs
    // if the home is shared
    if( m_oge_sharedHome )
    {
        env += ' ';
        env += m_envp;
    }

    return env;
}
//=============================================================================
string COgeMng::getDefaultNativeSpecification( const string &_queue, size_t _nJobs ) const
{

    // OGE specific settings
    // DRMAA_NATIVE_SPECIFICATION: there is an issue that will
    // be fixed in an upcoming release of DRMAA, that requires that
    // the native specification not start with whitespace.
    string nativeSpecification;
    // set queue
    // use default queue if parameter is empty
    string queue;
    if( _queue.empty() )
    {
        queueInfoContainer_t queues;
        getQueues( &queues );
        if( queues.empty() )
            throw runtime_error( "Can't find any resource queue." );

        queueInfoContainer_t::const_iterator found = find_if( queues.begin(),
                                                              queues.end(),
                                                              SFindQueue() );
        queue = ( queues.end() == found ) ? queues[0].m_name : found->m_name;

    }
    else
    {
        queue = _queue;
    }


    nativeSpecification += "-q ";
    nativeSpecification += queue;
    // export all environment vars.
    nativeSpecification += " -V ";
    // FIX: there is an issue with SGE 6.2, see bug ticket:
    // http://gridengine.sunsource.net/issues/show_bug.cgi?id=3261
    // if I submit a job with DRMAA or qsub (for qsub add the '-w v' option),
    // it fails with the error: "no suitable queues".
    // So I guess job verification is turned on by default for DRMAA for some reason.
    // Adding '-w n' to my DRMAA native specification makes things work for me
    nativeSpecification += " -w n ";
    // request tmp dir (needed by PoD workers) -- GSI specific
    // nativeSpecification += " -l tmp_free=100M ";
    // merge stdout and stderr
    nativeSpecification += " -j yes ";
    // output
    if( !m_server_logDir.empty() )
    {
        nativeSpecification += " -o ";
        nativeSpecification += m_server_logDir;
    }
    // TODO: add -@ if the user's option file is exists: "$POD_LOCATION/etc/Job.oge.options"
    string options_file( g_OGEOptionFile );
    smart_path( &options_file );
    if( does_file_exists( g_OGEOptionFile ) )
    {
        nativeSpecification += " -@ ";
        nativeSpecification += options_file;
    }

    return nativeSpecification;
}
//=============================================================================
string COgeMng::status2string( int _ogeJobStatus ) const
{
    switch( _ogeJobStatus )
    {
        case DRMAA_PS_UNDETERMINED:
            return ( "UNDETERMINED" ); //("Job status cannot be determined");
        case DRMAA_PS_QUEUED_ACTIVE:
            return ( "QUEUED_ACTIVE" ); //("Job is queued and active");
        case DRMAA_PS_SYSTEM_ON_HOLD:
            return ( "SYSTEM_ON_HOLD" ); //("Job is queued and in system hold");
        case DRMAA_PS_USER_ON_HOLD:
            return ( "USER_ON_HOLD" ); //("Job is queued and in user hold");
        case DRMAA_PS_USER_SYSTEM_ON_HOLD:
            return ( "USER_SYSTEM_ON_HOLD" ); //("Job is queued and in user and system hold");
        case DRMAA_PS_RUNNING:
            return ( "RUNNING" ); //("Job is running");
        case DRMAA_PS_SYSTEM_SUSPENDED:
            return ( "SYSTEM_SUSPENDED" ); //("Job is system suspended");
        case DRMAA_PS_USER_SUSPENDED:
            return ( "USER_SUSPENDED" ); //("Job is user suspended");
        case DRMAA_PS_USER_SYSTEM_SUSPENDED:
            return ( "USER_SYSTEM_SUSPENDED" ); //("Job is user and system suspended");
        case DRMAA_PS_DONE:
            return ( "DONE" ); //("Job finished normally");
        case DRMAA_PS_FAILED:
            return ( "FAILED" ); //("Job finished, but failed");
        default:
            return ( "UNKNOWN" );
    }
}
//=============================================================================
int COgeMng::jobStatus( const jobID_t &_id ) const
{
    int retval;
    if( !isValid( _id ) )
        return retval;

    try
    {
        initDRMAA();

        char error[DRMAA_ERROR_STRING_BUFFER];
        int errnum = 0;

        errnum = drmaa_job_ps( _id.c_str(), &retval, error,
                               DRMAA_ERROR_STRING_BUFFER );

        if( DRMAA_ERRNO_INVALID_JOB == errnum )
            retval = DRMAA_PS_DONE;
        else if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not get job' status [id=" );
            msg += _id;
            msg += "] :";
            msg += error;
            throw runtime_error( msg );
        }
    }
    catch( ... )
    {
        exitDRMAA();
        throw;
    }
    exitDRMAA();

    return retval;
}
//=============================================================================
void COgeMng::getQueues( queueInfoContainer_t *_container ) const
{
    if( !_container )
        return;

    _container->clear();

    try
    {
        // TODO: Check that the command executable exists
        string cmd( "$SGE_ROOT/bin/$SGE_ARCH/qconf" );
        smart_path( &cmd );

        StringVector_t args;
        args.push_back( "-sql" );

        string output;

        do_execv( cmd, args, 3/*3 secs timeout*/, &output );

        StringVector_t queueNames;
        stringstream ss( output );
        copy( istream_iterator<string>( ss ),
              istream_iterator<string>(),
              back_inserter( queueNames ) );

        StringVector_t::const_iterator iter = queueNames.begin();
        StringVector_t::const_iterator iter_end = queueNames.end();
        for( ; iter != iter_end; ++iter )
        {
            SQueueInfo info;
            info.m_name = *iter;

            _container->push_back( info );
        }
    }
    catch( ... /*const exception &_e*/ )
    {
        throw runtime_error( "Error retrieving queues information." );
    }
}
//=============================================================================
void COgeMng::killJob( const jobID_t &_id ) const
{
    char error[DRMAA_ERROR_STRING_BUFFER];
    int errnum = 0;
    try
    {
        initDRMAA();
        errnum = drmaa_control( _id.c_str(), DRMAA_CONTROL_TERMINATE, error,
                                DRMAA_ERROR_STRING_BUFFER );

        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not delete job [id=" );
            msg += _id;
            msg += "] :";
            msg += error;
            throw runtime_error( msg );
        }
    }
    catch( ... )
    {
        exitDRMAA();
        throw;
    }
    exitDRMAA();
}
//=============================================================================
bool COgeMng::isJobComplete( int _status )
{
    return (( DRMAA_PS_DONE == _status ) ||
            ( DRMAA_PS_FAILED == _status )
           );
}
//=============================================================================
void COgeMng::createJobsLogDir( const COgeMng::jobID_t &_parent ) const
{
    string path( m_server_logDir + getCleanParentID( _parent ) );
    // create a dir with read/write/search permissions for owner and group,
    // and with read/search permissions for others
    // TODO:Check for errors
    mkdir( path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
}
//=============================================================================
void COgeMng::setEnvironment( const string &_envp )
{
    m_envp = _envp;
}
