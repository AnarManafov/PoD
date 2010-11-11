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

        m_upload_log = _ud.getOptions().m_oge.m_uploadJobLog;
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
    if( jobID_t::npos != pos )
        return _parent;

    stringstream ss;
    ss << _parent << "." << _idx;
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

        // ===== Job Script
        errnum = drmaa_set_attribute( jt, DRMAA_REMOTE_COMMAND, _script.c_str(),
                                      error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set attribute " );
            msg += DRMAA_REMOTE_COMMAND;
            msg += ": ";
            msg += error;
            throw runtime_error( msg );
        }
        // ===== Set Native specifications
        string nativeSpecification( getDefaultNativeSpecification( _queue, _nJobs ) );
        errnum = drmaa_set_attribute( jt, DRMAA_NATIVE_SPECIFICATION, ( char * )nativeSpecification.c_str(),
                                      error, DRMAA_ERROR_STRING_BUFFER );
        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set attribute " );
            msg += DRMAA_NATIVE_SPECIFICATION;
            msg += ": ";
            msg += error;
            throw runtime_error( msg );
        }

        // DRMAA_V_ENV ( vector of strings )
        // The environment values that define the remote environment. Each string
        StringVector_t env( getEnvArray() );
        char **env_tmp = ( char** )malloc( sizeof( char* ) * ( env.size() + 1 ) );
        StringVector_t::const_iterator iter = env.begin();
        StringVector_t::const_iterator iter_end = env.end();
        size_t env_tmp_count = 0;
        for( ; iter != iter_end; ++iter, ++env_tmp_count )
        {
            env_tmp[env_tmp_count] = ( char* )malloc( sizeof( char ) * ( iter->size() + 1 ) );
            strcpy( env_tmp[env_tmp_count], iter->c_str() );
        }
        env_tmp[env_tmp_count] = NULL;

        errnum = drmaa_set_vector_attribute( jt, DRMAA_V_ENV, ( const char ** )env_tmp,
                                             error, DRMAA_ERROR_STRING_BUFFER );
        // delete temporary array of environment variables
        for( size_t i = 0; i < env_tmp_count; ++i )
        {
            free( env_tmp[i] );
        }
        free( env_tmp );

        if( errnum != DRMAA_ERRNO_SUCCESS )
        {
            string msg( "Could not set environment " );
            msg += DRMAA_V_ENV;
            msg += ": ";
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

            // creating a log dir for the job
            createJobsLogDir( ret[0] );
        }
        else
        {
            // something is wrong, we therefore remove the fake parent reservation
            ret.clear();
        }

        drmaa_release_job_ids( ids );
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
MiscCommon::StringVector_t COgeMng::getEnvArray() const
{
    StringVector_t env;
    string tmp;
    // set POD_UI_LOCATION on the worker nodes
    char *loc = getenv( "POD_LOCATION" );
    if( loc != NULL )
    {
        tmp = "POD_UI_LOCATION=";
        tmp += loc;
    }
    env.push_back( tmp );

    // set POD_UI_LOG_LOCATION variable on the worker nodes
    if( !m_server_logDir.empty() )
    {
        tmp = "POD_UI_LOG_LOCATION=";
        tmp += m_server_logDir;
        env.push_back( tmp );
    }

// We don't need to do the following commented code, since,
// SGE supports environment export, qsub -V.
// But we'll keep the code for a while just in case.
//
//    // export all env. variables of the process to jobs
//        size_t pos = m_envp.find( ',' );
//        size_t last_pos = 0;
//        for( ; pos != string::npos; )
//        {
//            tmp = m_envp.substr( last_pos, pos - last_pos );
//            env.push_back( tmp );
//
//            last_pos = pos + 1;
//            pos = m_envp.find( ',', last_pos );
//            if( string::npos == pos )
//            {
//                // assing the last key=value
//                tmp = m_envp.substr( last_pos, m_envp.size() - last_pos );
//            }
//    }

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
    if( !_queue.empty() )
    {
        nativeSpecification += "-q ";
        nativeSpecification += _queue;
    }

    // export all environment vars.
    nativeSpecification += " -V ";
    // FIX: there is an issue with SGE 6.2, see bug ticket:
    // http://gridengine.sunsource.net/issues/show_bug.cgi?id=3261
    // if I submit a job with DRMAA or qsub (for qsub add the '-w v' option),
    // it fails with the error: "no suitable queues".
    // So I guess job verification is turned on by default for DRMAA for some reason.
    // Adding '-w n' to my DRMAA native specification makes things work for me
    nativeSpecification += " -w n ";
    // By default DRMAA sets four options for all jobs.  These  are
    //    "-p  0", "-b yes", "-shell no", and "-w e".  This means that
    //    by default, all jobs will have priority 0, all jobs will  be
    //    treated  as binary, i.e. no scripts args will be parsed, all
    //    jobs will be executed without  a  wrapper  shell,  and  jobs
    //    which are unschedulable will cause a submit error.
    nativeSpecification += " -shell yes ";
    nativeSpecification += " -b no ";
    // request tmp dir (needed by PoD workers) -- GSI specific
    // nativeSpecification += " -l tmp_free=100M ";
    // merge stdout and stderr
    nativeSpecification += " -j yes ";
    // output
    if( m_upload_log )
    {
        if( !m_server_logDir.empty() )
        {
            nativeSpecification += " -o ";
            nativeSpecification += m_server_logDir;
        }
    }
    else
    {
        // send outputs to hell
        nativeSpecification += " -o /dev/null ";
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
    if( !m_upload_log )
        return;

    string path( m_server_logDir + _parent );
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
