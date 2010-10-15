/************************************************************************/
/**
 * @file OgeMng.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-10-13
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef OGEMNG_H_
#define OGEMNG_H_
// STD
#include <iosfwd>
#include <string>
#include <vector>
#include <map>
// MiscCommon
#include "PoDUserDefaultsOptions.h"

namespace oge_plug
{
    const int N_MAX_JOBS = 999;

    struct SNativeJobInfo
    {
        std::string m_status;
    };

    struct SQueueInfo
    {
        SQueueInfo(): m_maxJobs( N_MAX_JOBS )
        {}
        std::ostream& print( std::ostream &_stream ) const;

        // the queue name
        std::string m_name;
        // the maximum number of jobs that may be run in  the
        // queue concurrently
        // TODO: Didn't found yet how to get this info from DRMAA
        size_t m_maxJobs;
    };
    inline std::ostream &operator<<( std::ostream &_stream, const SQueueInfo &_info )
    {
        return _info.print( _stream );
    }

    //
    // This class uses DRMAA v1 to access API of OGE.
    //
    class COgeMng
    {
        public:
            typedef std::string jobID_t;
            typedef std::vector<jobID_t> jobArray_t;
//            typedef std::map<jobID_t, SNativeJobInfo> jobInfoContainer_t;
            typedef std::vector<SQueueInfo> queueInfoContainer_t;

        public:
//            void setUserDefaults( const PoD::CPoDUserDefaults &_ud );
            jobArray_t jobSubmit( const std::string &_script, const std::string &_queue,
                                  size_t _nJobs ) const;
//            std::string jobStatus( const jobID_t &_id ) const;
//            void jobStatusAllJobs( jobInfoContainer_t *_container ) const;
//            static std::string jobStatusToString( const std::string &_status );

            // Unfortunately DRMAA 1 doesn't support a requests
            // for a list of queues. It looks like it will be supported in v2.
            // We use "qconf -sql" to retrieve this information.
            void getQueues( queueInfoContainer_t *_container ) const;

//            void killJob( const jobID_t &_id ) const;
//            static size_t jobArrayStartIdx()
//            {
//                // job's array start index
//                return 0;
//            }
//
//            static bool isValid( const jobID_t &_id );
//            static jobID_t generateArrayJobID( const jobID_t &_parent, size_t _idx );
//            static bool isParentID( const jobID_t &_parent );
//            static bool isJobComplete( const std::string &_status );
//            void setEnvironment( const std::string &_envp );
//            std::string getCleanParentID( const jobID_t &_id ) const;

        private:
            void initDRMAA() const;
            void exitDRMAA() const;
//            void cleanAttr( attrl **attrib ) const;
//            void setDefaultPoDAttr( attrl **attrib, const std::string &_queue,
//                                    size_t _nJobs ) const;
            void createJobsLogDir( const jobID_t &_parent ) const;

        private:
            std::string m_server_logDir;
            bool m_pbs_sharedHome;
            std::string m_envp;
    };

};

#endif /* OGEMNG_H_ */
