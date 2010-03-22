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
}
// STD
#include <sstream>
#include <stdexcept>

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
bool CPbsMng::isValid( const CPbsMng::jobID_t &_id )
{
    return !_id.empty();
}
//=============================================================================
CPbsMng::jobID_t CPbsMng::jobSubmit( const std::string &_cmd )
{
    // Connect to the pbs server
    // We use NULL as a PBS server string, a connection will be
    // opened to the default server.
    int connect = pbs_connect( NULL );
    if ( connect < 0 )
        throw pbs_error( "Error occurred while connecting to pbs server." );


    attrl attrib;

    // queue a job request
    char script[] = "SCRIPT.PBS";
    // the destination (4th parameter) will be provided via attributes - ATTR_queue
    char *jobid = pbs_submit( connect, reinterpret_cast<attropl*>( &attrib ),
                              script, NULL, NULL );
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
