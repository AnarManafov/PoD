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

typedef std::vector<SJobInfo *> jobs_children_t;


struct SJobInfo
{
    SJobInfo():
            m_id( 0 ),
            m_status( CLsfMng::JS_JOB_STAT_UNKWN ),
            m_parent(NULL)
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
        m_children.push_back( _child );
    }
    int indexOf (const SJobInfo *_info) const
    {
    	// TODO: find a faster algorithm
    	jobs_children_t::const_iterator iter = std::find( m_children.begin(), m_children.end(), _info );
        return std::distance( m_children.begin(), iter );
    }
    int row() const
    {
    	if( m_parent )
    	{
    		//SJobInfo *info = const_cast<SJobInfo*>(m_parent);
    		return m_parent->indexOf( this );
    	}

    	return -1;
    }

    lsf_jobid_t m_id;
    std::string m_strID;
    CLsfMng::EJobStatus_t m_status;
    std::string m_strStatus;
    jobs_children_t m_children;
    SJobInfo *m_parent; //!< parent of this job or NULL
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
};

#endif /* JOBINFO_H_ */
