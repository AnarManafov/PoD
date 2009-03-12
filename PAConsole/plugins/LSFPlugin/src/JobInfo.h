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
#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
// STD
#include <algorithm>
// lsf plug-in
#include "LSFJobSubmitter.h"

#include <iostream>

struct SJobInfo;

typedef boost::shared_ptr<SJobInfo> SJobInfoPTR_t;
//typedef std::map<lsf_jobid_t, SJobInfoPTR_t> JobsContainer_t;
typedef std::map<std::string, SJobInfoPTR_t> JobsContainer_t;
typedef std::vector<SJobInfoPTR_t> jobs_children_t;


struct SJobInfo
{
    SJobInfo():
            m_id( 0 ),
            m_status( CLsfMng::JS_JOB_STAT_UNKWN ),
       m_parent(NULL),
       m_index(-1)
    {}

  SJobInfo& operator=(const SJobInfo &_info)
  {
    if (this != &_info)
      {
	m_id = _info.m_id;
	m_strID = _info.m_strID;
	m_status = _info.m_status;
	m_strStatus = _info.m_strStatus;
	// keep the old list of children and the parent reference (ee the following TODO)
	// TODO: very ugly. Revise that.
	//m_children = _info.m_children;
	//m_parent = _info.m_parent;
	m_index = _info.m_index;
      }
    return *this;
  }
  
  ~SJobInfo()
    {
    }
    bool operator ==( const SJobInfo &_info )
    {
        if ( m_strID != _info.m_strID )
            return false;
        if ( m_status != _info.m_status )
            return false;

        return true;
    }
    int addChild( SJobInfoPTR_t _child )
    {
        m_children.push_back( _child );
	return m_children.size() - 1;
    }
    int indexOf (const SJobInfo *_info) const
    {
        // TODO: find a faster algorithm
      //  SJobInfoPTR_t p( const_cast<SJobInfo*>(_info) );
      //  jobs_children_t::const_iterator iter = std::find( m_children.begin(), m_children.end(),
      //                  p );
  //  return std::distance( m_children.begin(), iter );
      return _info->m_index; 
    }
    int row() const
    {
        if ( m_parent )
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
    int m_index;
};

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
    void addChildItem( lsf_jobid_t _JobID, SJobInfoPTR_t _parent );

private:
    JobsContainer_t m_Container;
    const CLsfMng &m_lsf;
};

#endif /* JOBINFO_H_ */
