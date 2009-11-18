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

        Copyright (c) 2008-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFMNG_H_
#define LSFMNG_H_
//=============================================================================
// LSF API
#include <lsf/lsbatch.h>
// STD
#include <string>
#include <map>
#include <vector>
// Qt
#include <QMetaType>
// MiscCommon
#include "def.h"
//=============================================================================
typedef long long int lsf_jobid_t;
//=============================================================================
typedef struct _SLSFQueueInfo
{
    _SLSFQueueInfo(): m_userJobLimit( 100 )
    {}

    int m_userJobLimit;

} SLSFQueueInfo_t;
//=============================================================================
// making Qt to know this data type
// in order to use it in QVariant, for example
Q_DECLARE_METATYPE( SLSFQueueInfo_t )
//=============================================================================
// key = <queue name>, value = SLSFQueueInfo_t
typedef std::map<std::string, SLSFQueueInfo_t> LSFQueueInfoMap_t;
//=============================================================================
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
        } EJobProperty_t;

        // possible values for the status field
        typedef enum EJobStatus
        {
            JS_JOB_STAT_PEND   = 0x01,    //!< job is pending
            JS_JOB_STAT_PSUSP  = 0x02,    //!< job is held
            JS_JOB_STAT_RUN    = 0x04,    //!< job is running
            JS_JOB_STAT_SSUSP  = 0x08,    //!< job is suspended by LSF Batch system
            JS_JOB_STAT_USUSP = 0x10,    //!< job is suspended by user
            JS_JOB_STAT_EXIT = 0x20,    //!< job exited
            JS_JOB_STAT_DONE = 0x40,    //!< job is completed successfully
            JS_JOB_STAT_PDONE = 0x80,    //!< post job process done successfully
            JS_JOB_STAT_PERROR = 0x100,   //!< post job process error
            JS_JOB_STAT_WAIT = 0x200,    //!< chunk job waiting its execution turn
            JS_JOB_STAT_UNKWN = 0x1000,   //!< unknown status
            JS_JOB_STAT_COMPLETED = 0x2000 //!< a custom status from PoD. Means that the job was completed and there is no need to monitor it.
        } EJobStatus_t;

        typedef std::map<EJobProperty_t, std::string> propertyDict_t;
        typedef std::vector<lsf_jobid_t> IDContainer_t;

    public:
        CLsfMng();
        virtual ~CLsfMng();

    public:
        void init();
        void addProperty( EJobProperty_t _type, const std::string &_val );
        // TODO: implement
        //void removeProperty();
        lsf_jobid_t jobSubmit( const std::string &_Cmd );
        EJobStatus_t jobStatus( lsf_jobid_t _jobID ) const;
        std::string jobStatusString( lsf_jobid_t _jobID ) const;
        std::string jobStatusString( CLsfMng::EJobStatus_t _jobStatus ) const;
        int getNumberOfChildren( lsf_jobid_t _jobID ) const;
        void getChildren( lsf_jobid_t _jobID, IDContainer_t *_container ) const;
        void getQueues( LSFQueueInfoMap_t *_retVal ) const;
        void killJob( lsf_jobid_t _jobID ) const;

    private:
        propertyDict_t m_submitRequest;
        bool m_bInit;
};

#endif /* LSFMNG_H_ */
