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
// MiscCommon
#include "def.h"

struct attrl;

struct SJobInfo
{
    std::string m_status;
};

class CPbsMng
{
    public:
        typedef std::string jobID_t;
        typedef std::vector<jobID_t> jobArray_t;
        typedef std::map<jobID_t, SJobInfo> jobInfoContainer_t;

    public:
        bool isValid( const jobID_t &_id ) const;
        jobArray_t jobSubmit( const std::string &_script, const std::string &_queue,
                              size_t _nJobs,
                              const std::string &_outputPath ) const;
        std::string jobStatus( const jobID_t &_id ) const;
        void jobStatusAllJobs( jobInfoContainer_t *_container,
                               const jobArray_t &_ids ) const;
        static std::string jobStatusToString( const char &_status );
        void getQueues( MiscCommon::StringVector_t *_container ) const;

    private:
        void cleanAttr( attrl **attrib ) const;
        void setDefaultPoDAttr( attrl **attrib, const std::string &_queue,
                                size_t _nJobs,
                                const std::string &_outputPath ) const;
        jobID_t generateArrayJobID( const jobID_t &_parent, size_t _idx ) const;
};

#endif /* PBSMNG_H_ */
