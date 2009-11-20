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

        typedef std::map<EJobProperty_t, std::string> propertyDict_t;
        typedef std::vector<lsf_jobid_t> IDContainer_t;
        // the second (value) is a status of the job
        typedef std::map<lsf_jobid_t, int> IDContainerOrdered_t;

    public:
        CLsfMng();
        virtual ~CLsfMng();

    public:
        void init();
        void addProperty( EJobProperty_t _type, const std::string &_val );
        // TODO: implement
        //void removeProperty();
        lsf_jobid_t jobSubmit( const std::string &_Cmd );
        static std::string jobStatusString( int _status );
        int getNumberOfChildren( lsf_jobid_t _jobID ) const;
        void getChildren( lsf_jobid_t _jobID, IDContainer_t *_container ) const;
        void getQueues( LSFQueueInfoMap_t *_retVal ) const;
        void killJob( lsf_jobid_t _jobID ) const;
        size_t getAllUnfinishedJobs( IDContainerOrdered_t *_container ) const;

    private:
        propertyDict_t m_submitRequest;
        bool m_bInit;
        std::string m_user;
};

#endif /* LSFMNG_H_ */
