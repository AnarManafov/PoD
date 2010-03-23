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

using namespace std;
//=============================================================================
class pbs_error: public std::exception
{
    public:
        explicit pbs_error( const std::string &_ErrorPrefix )
        {
            m_errno = pbs_errno;
            std::stringstream ss;
            if ( !_ErrorPrefix.empty() )
                ss << _ErrorPrefix << " ";
            ss <<  "PBS error [" << m_errno << "]: " << pbs_strerror( pbs_errno );
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
        std::string m_Msg;
        int m_errno;
};
//=============================================================================
bool CPbsMng::isValid( const CPbsMng::jobID_t &_id ) const
{
    return !_id.empty();
}
//=============================================================================
CPbsMng::jobID_t CPbsMng::jobSubmit( const string &_script, const string &_queue,
                                     size_t _nJobs,
                                     const string &_outputPath ) const
{
    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if ( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );


    // TODO: don't forget to call free
    attrl *attrib = NULL;

    // Set default attributes for PoD
    setDefaultPoDAttr( &attrib, _queue, _nJobs, _outputPath );

    // The destination (4th parameter) will be provided via attributes - ATTR_queue
    // HACK: I hate to use const_cast, but
    // for some unknown reason API developers wants char* and not const char * const
    char *jobid = pbs_submit( connect, reinterpret_cast<attropl*>( &attrib ),
                              const_cast<char*>( _script.c_str() ), NULL, NULL );
    cleanAttr( &attrib );

    if ( NULL == jobid )
    {
        // get error message and disconnect
        char* errmsg = pbs_geterrmsg( connect );
        pbs_disconnect( connect );

        if ( errmsg != NULL )
        {
            string msg( "Error submitting job." );
            msg += errmsg;
            throw runtime_error( msg );
        }
        throw pbs_error( "Error submitting job." );
    }

    // close the connection with the server
    pbs_disconnect( connect );

    // return job id
    jobID_t ret( jobid );
    free( jobid );
    return ret;
}
//=============================================================================
void CPbsMng::cleanAttr( attrl **attrib ) const
{
    attrl *next = *attrib;
    while ( next != NULL )
    {
        attrl *del = next;
        next = del->next;
        
        free( del->name );
        free( del->resource );
        free( del->value );
        free( del );
    }
}
//=============================================================================
void CPbsMng::setDefaultPoDAttr( attrl **attrib, const string &_queue,
                                 size_t _nJobs, const string &_outputPath ) const
{
    // job's name
    set_attr( attrib, ATTR_N, "PoD" );
    // join error/stdoutput
    set_attr( attrib, ATTR_j, "oe" );
    // queue (destination)
    set_attr( attrib, ATTR_q, _queue.c_str() );
    // an array job
    stringstream ss;
    ss << "1-" << _nJobs;
    set_attr( attrib, ATTR_t, ss.str().c_str() );
    // output path
    set_attr( attrib, ATTR_o, _outputPath.c_str() );
}
