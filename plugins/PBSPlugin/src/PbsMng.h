/************************************************************************/
/**
 * @file PbsMng.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-22
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PBSMNG_H_
#define PBSMNG_H_
// STD
#include <string>
#include <vector>
#include <map>

struct attrl;

namespace pbs_plug
{
    struct _SJobInfo
    {
        std::string m_status;
    };

    struct SQueueInfo
    {
        // the queue name
        std::string m_name;
        // the maximum number of jobs that may be run in  the
        // queue concurrently
        size_t m_maxJobs;
    };
    inline std::ostream & operator<<( std::ostream &_stream, const SQueueInfo &_info )
    {
        return _stream;
    }

    class CPbsMng
    {
        public:
            typedef std::string jobID_t;
            typedef std::vector<jobID_t> jobArray_t;
            typedef std::map<jobID_t, _SJobInfo> jobInfoContainer_t;
            typedef std::vector<SQueueInfo> queueInfoContainer_t;

        public:
            bool isValid( const jobID_t &_id ) const;
            jobArray_t jobSubmit( const std::string &_script, const std::string &_queue,
                                  size_t _nJobs,
                                  const std::string &_outputPath ) const;
            std::string jobStatus( const jobID_t &_id ) const;
            void jobStatusAllJobs( jobInfoContainer_t *_container,
                                   const jobArray_t &_ids ) const;
            static std::string jobStatusToString( const char &_status );
            void getQueues( queueInfoContainer_t *_container ) const;

        private:
            void cleanAttr( attrl **attrib ) const;
            void setDefaultPoDAttr( attrl **attrib, const std::string &_queue,
                                    size_t _nJobs,
                                    const std::string &_outputPath ) const;
            jobID_t generateArrayJobID( const jobID_t &_parent, size_t _idx ) const;
    };

};

#endif /* PBSMNG_H_ */
