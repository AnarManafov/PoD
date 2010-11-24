/************************************************************************/
/**
 * @file PbsMng.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// PBS plug-in
#include "PbsMng.h"
// API
#include <sys/stat.h>
// PBS API
extern "C"
{
#include "pbs_error.h"
#include "pbs_ifl.h"
    void set_attr(
        struct attrl **attrib,        /* I */
        const char * const attrib_name,   /* I */
        const char * const attrib_value );
}
// STD
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
// Qt
#include <QtGlobal>
// MiscCommon
#include "SysHelper.h"

using namespace std;
using namespace pbs_plug;
using namespace MiscCommon;
//=============================================================================
ostream &SQueueInfo::print( ostream &_stream ) const
{
    _stream
            << "queue name: " << m_name
            << "; max jobs: " << m_maxJobs
            << "\n";

    return _stream;
}
//=============================================================================
class pbs_error: public exception
{
    public:
        explicit pbs_error( const string &_ErrorPrefix )
        {
            m_errno = pbs_errno;
            stringstream ss;
            if( !_ErrorPrefix.empty() )
                ss << _ErrorPrefix << " ";
#if defined(HAVE_PBSE_TO_TXT)
            ss <<  "PBS error [" << m_errno << "]: " << pbse_to_txt( pbs_errno );
#else
            ss <<  "PBS error [" << m_errno << "]: " << pbs_strerror( pbs_errno );
#endif
            m_Msg = ss.str();
        }
        virtual ~pbs_error() throw()
        {}
        virtual const char* what() const throw()
        {
            return m_Msg.c_str();
        }
        int getErrno() const throw()
        {
            return m_errno;
        }

    private:
        string m_Msg;
        int m_errno;
};
//=============================================================================
void CPbsMng::setUserDefaults( const PoD::CPoDUserDefaults &_ud )
{
    try
    {
        m_server_logDir = _ud.getValueForKey( "server.logfile_dir" );
        smart_path( &m_server_logDir );
        smart_append( &m_server_logDir, '/' );

        m_serverWrkDir = _ud.getOptions().m_server.m_common.m_workDir;
        smart_path( &m_serverWrkDir );
        smart_append( &m_serverWrkDir, '/' );

        m_upload_log = _ud.getOptions().m_pbs.m_uploadJobLog;
    }
    catch( exception &e )
    {
    }
}
//=============================================================================
bool CPbsMng::isValid( const jobID_t &_id )
{
    return !_id.empty();
}
//=============================================================================
string CPbsMng::getCleanParentID( const jobID_t &_id ) const
{
    // JobID in OpenPBS: PARENTID.server.fqdn PARENTID-ARRAYINDEX.server.fqdn
    // JobID in PBSPro: PARENTID[].server.fqdn PARENTID[ARRAYINDEX].server.fqdn
    // Clean ParentID is the PARENTID parent of these ids.
    jobID_t::size_type pos = _id.find_first_of( ".[" );
    if( jobID_t::npos == pos )
        return _id;

    return _id.substr( 0, pos );
}
//=============================================================================
bool CPbsMng::isParentID( const jobID_t &_parent )
{
    jobID_t::size_type pos = _parent.find_first_of( ".[" );
    if( jobID_t::npos == pos )
        return false; // Bad id?

    string str( _parent, 0, pos );
    return ( str.find( '-' ) == string::npos );
}
//=============================================================================
CPbsMng::jobID_t CPbsMng::generateArrayJobID( const jobID_t &_parent,
                                              size_t _idx )
{
    // to get an array ID we need to add "-index" to a parentID, to get:
    // SERVERID-INDEX.SERVER
    jobID_t::size_type pos = _parent.find_first_of( ".[" );
    if( jobID_t::npos == pos )
        return _parent;

    stringstream ss;
    ss << "-" << _idx;

    string ret( _parent );
    ret.insert( pos, ss.str() );
    return ret;
}
//=============================================================================
CPbsMng::jobArray_t CPbsMng::jobSubmit( const string &_script, const string &_queue,
                                        size_t _nJobs ) const
{
    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );

    // TODO: don't forget to call free
    attrl *attrib = NULL;

    // Set default attributes for PoD
    setDefaultPoDAttr( &attrib, _queue, _nJobs );

    qDebug( "pbs call: job submit" );
    // The destination (4th parameter) will be provided via attributes - ATTR_
    // HACK: I hate to use const_cast, but
    // for some unknown reason API developers wants char* and not const char * const
    char *jobid = pbs_submit( connect, reinterpret_cast<attropl*>( attrib ),
                              const_cast<char*>( _script.c_str() ), NULL, NULL );

    cleanAttr( &attrib );

    if( NULL == jobid )
    {
        // get error message and disconnect
        char* errmsg = pbs_geterrmsg( connect );
        pbs_disconnect( connect );

        if( errmsg != NULL )
        {
            string msg( "Error submitting job." );
            msg += errmsg;
            throw runtime_error( msg );
        }
        throw pbs_error( "Error submitting job." );
    }

    // close the connection with the server
    pbs_disconnect( connect );

    // creating a log dir for the job
    createJobsLogDir( jobid );

    // return job id
    // return an array of jobs id.
    // the first element is the "parent" job id - a fake parent
    // all the rest is array jobs ids
    jobArray_t ret;
    ret.push_back( jobid );
    for( size_t i = jobArrayStartIdx(); i < _nJobs; ++i )
    {
        jobID_t id = generateArrayJobID( jobid, i );
        ret.push_back( id );
    }
    free( jobid );

    return ret;
}
//=============================================================================
void CPbsMng::cleanAttr( attrl **attrib ) const
{
    attrl *next = *attrib;
    while( next != NULL )
    {
        attrl *del = next;
        next = del->next;

        free( del->name );
        del->name = NULL;
        free( del->resource );
        del->resource = NULL;
        free( del->value );
        del->value = NULL;
        free( del );
        del = NULL;
    }
}
//=============================================================================
void CPbsMng::setDefaultPoDAttr( attrl **attrib, const string &_queue,
                                 size_t _nJobs ) const
{
    // TODO: check plug-in setting for "pbs shared home", before adding a host to output path
    // PBS needs a fullpath including an UI host name
    // Currently we assume that our UI is the machine where
    // pod-console has been started
    string output( m_server_logDir );
    string hostname;
    MiscCommon::get_hostname( &hostname );
    output = hostname + ":" + output;

    // job's name
    set_attr( attrib, ATTR_N, "PoD" );
    // join error/stdoutput
    set_attr( attrib, ATTR_j, "oe" );
    // queue (desti nation)
    set_attr( attrib, ATTR_queue, _queue.c_str() );
    // an array job
    stringstream ss;
    ss << jobArrayStartIdx() << "-" << ( jobArrayStartIdx() + _nJobs - 1 );
    set_attr( attrib, ATTR_t, ss.str().c_str() );
    // output path
    if( m_upload_log )
        set_attr( attrib, ATTR_o, output.c_str() );
    else
        set_attr( attrib, ATTR_o, "/dev/null" );

    // set additional environment variables
    string env;
    // set POD_UI_LOCATION on the worker nodes
    char *loc = getenv( "POD_LOCATION" );
    if( loc != NULL )
    {
        env += "POD_UI_LOCATION=";
        env += loc;
        env += ',';
    }
    // set POD_UI_LOG_LOCATION variable on the worker nodes
    env += "POD_UI_LOG_LOCATION=";
    env += m_server_logDir;
    // set POD_SRV_WORKDIR variable on the worker nodes
    env += ',';
    env += "POD_SRV_WORKDIR=";
    env += m_serverWrkDir;


    // export all env. variables of the process to jobs
    // if the home is shared
    if( !m_envp.empty() )
    {
        env += ',';
        env += m_envp;
    }

    set_attr( attrib, ATTR_v, env.c_str() );
}
//=============================================================================
string CPbsMng::jobStatus( const jobID_t &_id ) const
{
    string retval;
    if( _id.empty() )
        return retval;

    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );

    batch_status *p_status = NULL;

    qDebug( "pbs call: job status" );
    p_status = pbs_statjob( connect, const_cast<char*>( _id.c_str() ),
                            NULL, const_cast<char*>( EXECQUEONLY ) );
    if( NULL == p_status && 0 != pbs_errno )
    {
        // close the connection with the server
        pbs_disconnect( connect );
        throw pbs_error( "Error getting job's status." );
    }

    attrl *a( p_status->attribs );
    while( a != NULL )
    {
        if( NULL == a->name )
            break;

        if( !strcmp( a->name, ATTR_state ) )
        {
            retval = a->value;
            break;
        }

        a = a->next;
    }

    pbs_statfree( p_status );

    // close the connection with the server
    pbs_disconnect( connect );
    return retval;
}
//=============================================================================
void CPbsMng::jobStatusAllJobs( CPbsMng::jobInfoContainer_t *_container ) const
{
    if( !_container )
        return;

    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );

    batch_status *p_status = NULL;

    qDebug( "pbs call: job status" );
    // request information of all jobs
    p_status = pbs_statjob( connect, NULL, NULL, NULL );
    if( NULL == p_status && 0 != pbs_errno )
    {
        // close the connection with the server
        pbs_disconnect( connect );
        throw pbs_error( "Error getting job's status." );
    }

    batch_status *p( NULL );
    for( p = p_status; p != NULL; p = p->next )
    {
        string id = p->name;

        attrl *a( p->attribs );
        while( a != NULL )
        {
            if( NULL == a->name )
                break;

            if( !strcmp( a->name, ATTR_state ) )
            {
                SNativeJobInfo info;
                info.m_status = a->value;

                _container->insert( jobInfoContainer_t::value_type( id, info ) );
            }

            a = a->next;
        }
    }

    pbs_statfree( p_status );

    // close the connection with the server
    pbs_disconnect( connect );
}
//=============================================================================
void CPbsMng::getQueues( queueInfoContainer_t *_container ) const
{
    if( !_container )
        return;

    _container->clear();

    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );

    batch_status *p_status = NULL;

    // request information of all queues
    p_status = pbs_statque( connect, NULL, NULL, NULL );
    if( NULL == p_status )
    {
        // close the connection with the server
        pbs_disconnect( connect );
        throw pbs_error( "Error getting queues status." );
    }

    batch_status *p( NULL );
    for( p = p_status; p != NULL; p = p->next )
    {
        SQueueInfo info;
        info.m_name = p->name;

        attrl *a( p->attribs );
        while( a != NULL )
        {
            if( NULL == a->name )
                break;

            // job number limit
            if( !strcmp( a->name, ATTR_maxrun ) )
            {
                istringstream ss( a->value );
                ss >> info.m_maxJobs;
                // I think, that 0 means unset,
                // therefore I set it to maximum in this case
                if( 0 == info.m_maxJobs )
                    info.m_maxJobs = N_MAX_JOBS;

                break;
            }

            a = a->next;
        }

        _container->push_back( info );
    }

    pbs_statfree( p_status );

    // close the connection with the server
    pbs_disconnect( connect );
}
//=============================================================================
void CPbsMng::killJob( const jobID_t &_id ) const
{
    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );

    qDebug( "pbs call: job kill" );
    if( 0 != ( pbs_deljob( connect, const_cast<char*>( _id.c_str() ), NULL ) ) )
    {
        // close the connection with the server
        pbs_disconnect( connect );
        throw pbs_error( "Error killing the job." );
    }

    // close the connection with the server
    pbs_disconnect( connect );
}
//=============================================================================
string CPbsMng::jobStatusToString( const string &_status )
{
    if( _status.empty() )
        return "unknown";

    // PBS's status job is just a char so far.
    // we use string as a parameter in case we decide to change the algorithms
    switch( _status[0] )
    {
        case 'T': // Job is in transition (being moved to a new location)
            return "transit";
        case 'Q': // Job is queued, eligible to run or be routed
            return "queued";
        case 'H': // Job is held
            return "held";
        case 'W': // Job is waiting for its requested execution time to be reached,
            // or the job’s specified stagein request has failed for some reason
            return "waiting";
        case 'R': // Job is running
            return "running";
        case 'E': // Job is exiting after having run
            return "exiting";
        case 'C':
            return "complete";
        case 'B': // Job arrays only: job array has started
            return "started";
        case 'S': // Job is suspended by server
            return "suspended";
        case 'U': // Job is suspended due to workstation becoming busy
            return "suspended by keyboard activity";
        case 'X': // Subjobs only; subjob is finished (expired.)
            return "finished";
        default:
            return "unknown";
    }
}
//=============================================================================
bool CPbsMng::isJobComplete( const string &_status )
{
    if( _status.empty() )
        return false;

    return ( 'C' == _status[0] );
}
//=============================================================================
void CPbsMng::createJobsLogDir( const CPbsMng::jobID_t &_parent ) const
{
    if( !m_upload_log )
        return;

    string path( m_server_logDir + getCleanParentID( _parent ) );
    // create a dir with read/write/search permissions for owner and group,
    // and with read/search permissions for others
    // TODO:Check for errors
    mkdir( path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
}
//=============================================================================
void CPbsMng::setEnvironment( const string &_envp )
{
    m_envp = _envp;
}
