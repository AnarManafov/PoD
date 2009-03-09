/************************************************************************/
/**
 * @file JobInfo.h
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-06
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
#ifndef JOBINFO_H_
#define JOBINFO_H_

// BOOST
#include <boost/shared_ptr.hpp>
// lsf plug-in
#include "LSFJobSubmitter.h"


struct SJobInfo;

typedef boost::shared_ptr<SJobInfo> SJobInfoPTR_t;
typedef std::vector<SJobInfoPTR_t> jobs_children_t;

struct SJobInfo
{
    SJobInfo():
            m_id( 0 ),
            m_status( CLsfMng::JS_JOB_STAT_UNKWN ),
            m_parent(NULL),
            m_index(0)
    {}
    bool operator ==( const SJobInfo &_info )
    {
        if ( m_id != _info.m_id )
            return false;
        if ( m_status != _info.m_status )
            return false;

        return true;
    }
    void addChild( SJobInfo *_child )
    {
        m_children.push_back( SJobInfoPTR_t(_child) );
    }

    lsf_jobid_t m_id;
    std::string m_strID;
    CLsfMng::EJobStatus_t m_status;
    std::string m_strStatus;
    jobs_children_t m_children;
    SJobInfo *m_parent; //!< parent of this job or NULL
    size_t m_index;
};


typedef std::map<lsf_jobid_t, SJobInfo> JobsContainer_t;
/**
 *
 *  this comparison operator is required to use the container with generic algorithms
 *
 */
inline bool operator <( const JobsContainer_t::value_type &_v1,
                        const JobsContainer_t::value_type &_v2 )
{
    return _v1.first < _v2.first;
}

class CJobInfo
{
public:
    CJobInfo(const CLsfMng &_lsf);
    virtual ~CJobInfo();

public:
    void update( const CLSFJobSubmitter::jobslist_t &_Jobs, JobsContainer_t *_Container = NULL );
    void getInfo( JobsContainer_t *_Container );

private:
	void addChildItem( lsf_jobid_t _JobID, SJobInfo *_parent );

private:
    JobsContainer_t m_Container;
    const CLsfMng &m_lsf;
    size_t m_itemsMaxNumber;
};

#endif /* JOBINFO_H_ */
