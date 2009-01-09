/************************************************************************/
/**
 * @file LsfMng.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-12-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFMNG_H_
#define LSFMNG_H_

// LSF API
#include <lsf/lsbatch.h>
// STD
#include <string>
#include <map>
#include <vector>

typedef long long int LS_LONG_INT_t;

class CLsfMng
{
    public:
        typedef enum EJobProperty
        {
            JP_SUB_JOB_NAME,  //!< job name specified
            JP_SUB_QUEUE,   //!< queue specified
            JP_SUB_HOST,   //!< execution host(s) specified
            JP_SUB_IN_FILE,   //!< input file specified
            JP_SUB_OUT_FILE,  //!< output file specified
            JP_SUB_ERR_FILE   //!< error file specified
        }EJobProperty_t;

        // possible values for the status field
        typedef enum EJobStatus
        {
            JS_JOB_STAT_PEND, 		//!< job is pending
            JS_JOB_STAT_PSUSP, 		//!< job is held
            JS_JOB_STAT_RUN, 		//!< job is running
            JS_JOB_STAT_SSUSP, 		//!< job is suspended by LSF Batch system
            JS_JOB_STAT_USUSP, 		//!< job is suspended by user
            JS_JOB_STAT_EXIT, 		//!< job exited
            JS_JOB_STAT_DONE, 		//!< job is completed successfully
            JS_JOB_STAT_PDONE, 		//!< post job process done successfully
            JS_JOB_STAT_PERROR,		//!< post job process error
            JS_JOB_STAT_WAIT, 		//!< chunk job waiting its execution turn
            JS_JOB_STAT_UNKWN, 		//!< unknown status
        } EJobStatus_t;

        typedef std::map<EJobProperty_t, std::string> propertyDict_t;
        typedef std::vector<LS_LONG_INT_t> IDContainer_t;

    public:
        CLsfMng();
        virtual ~CLsfMng();

    public:
        void init();
        void addProperty( EJobProperty_t _type, const std::string &_val );
        // TODO: implement
        //void removeProperty();
        LS_LONG_INT_t jobSubmit( const std::string &_Cmd );
        EJobStatus_t jobStatus( LS_LONG_INT_t _jobID );
        std::string jobStatusString( LS_LONG_INT_t _jobID );
        int getNumberOfChildren( LS_LONG_INT_t _jobID ) const;
        void getChildren( LS_LONG_INT_t _jobID, IDContainer_t *_container ) const;

    private:
        propertyDict_t m_submitRequest;
        bool m_bInit;
};

#endif /* LSFMNG_H_ */
