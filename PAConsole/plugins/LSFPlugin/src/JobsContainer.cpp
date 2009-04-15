/************************************************************************/
/**
 * @file JobsContainer.cpp
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-06
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/bind.hpp>
// LSF plug-in
#include "JobsContainer.h"

using namespace std;

QWaitCondition g_signalIsPosted;
QMutex mutex;

CJobsContainer::CJobsContainer( const CLSFJobSubmitter *_lsfsubmitter):
        m_lsfsubmitter(_lsfsubmitter),
        m_jobInfo(_lsfsubmitter->getLSF()),
        m_updateNumberOfJobs(true),
        m_updateInterval(0)
{
  // we need to register SJobInfoPTR_t, so that Qt will be able to
  // marshal this type
   qRegisterMetaType<SJobInfoPTR_t>("SJobInfoPTR_t");
}

CJobsContainer::~CJobsContainer()
{
    // stop the thread
    unsigned long wait_time = m_updateInterval * 2;
    m_updateInterval = 0;
    wait( wait_time );
    if ( isRunning() )
        terminate();
}

void CJobsContainer::update( long _update_time_ms )
{
    m_updateInterval = _update_time_ms;

    if ( !isRunning() )
        start();
}

void CJobsContainer::run()
{
  forever
    {
      if ( 0 == m_updateInterval )
	return;
      
      if (m_updateNumberOfJobs)
        {
	  _updateNumberOfJobs();
	  m_updateNumberOfJobs = false;
        }
      else
	_updateJobsStatus();
      
      msleep(m_updateInterval);
    }
}

void CJobsContainer::updateNumberOfJobs()
{
    m_updateNumberOfJobs = true;
}

void CJobsContainer::_updateNumberOfJobs()
{
    JobsContainer_t newinfo;
    m_jobInfo.update( m_lsfsubmitter->getActiveJobList(), &newinfo );

    // adding all jobs for the first time
    if ( m_curinfo.empty() )
    {
        for_each( newinfo.begin(), newinfo.end(),
                  boost::bind( &CJobsContainer::_addJobInfo, this, _1 ) );
        return;
    }

    // Checking jobs for removal
    JobsContainer_t tmp;
    set_difference( m_cur_ids.begin(), m_cur_ids.end(),
                    newinfo.begin(), newinfo.end(),
                    inserter( tmp, tmp.begin() ));

    // Delete children first
    JobsContainer_t::const_iterator iter = tmp.begin();
    JobsContainer_t::const_iterator iter_end = tmp.end();
    for(; iter != iter_end; ++iter)
      {
	if( NULL != iter->second->m_parent && 0 != iter->second->m_parent->m_id )
	  _removeJobInfo( *iter );
      }
    // Delete parents
    iter = tmp.begin();
    for(; iter != iter_end; ++iter)
      {
	if( NULL == iter->second->m_parent || 0 == iter->second->m_parent->m_id )
	  _removeJobInfo( *iter );
      }

    // Checking for newly added jobs
    tmp.clear();
    set_difference( newinfo.begin(), newinfo.end(),
                    m_cur_ids.begin(), m_cur_ids.end(),
                    inserter( tmp, tmp.begin() ) );
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_addJobInfo, this, _1 ) );
}

void CJobsContainer::_updateJobsStatus()
{
    for_each( m_curinfo.begin(), m_curinfo.end(),
              boost::bind( &CJobsContainer::_updateJobInfo, this, _1 ) );
}

void CJobsContainer::_addJobInfo( const JobsContainer_t::value_type &_node )
{
    SJobInfoPTR_t info = _node.second;

    pair<JobsContainer_t::iterator, bool> res =  m_cur_ids.insert( JobsContainer_t::value_type( info->m_strID, info ) );

    if ( res.second )
    {
        // parent jobs
      emit addJob( info );
      m_curinfo.insert( JobsContainer_t::value_type( info->m_strID, info ) );
      m_container.push_back( info.get() );
    }

    // adding children to the model
    jobs_children_t::const_iterator iter = info.get()->m_children.begin();
    jobs_children_t::const_iterator iter_end = info.get()->m_children.end();
    for (; iter != iter_end; ++iter)
    {
        res = m_cur_ids.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
        if ( res.second )
        {
	  emit addJob( *iter );
	  m_curinfo.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
        }
    }
}

void CJobsContainer::_removeJobInfo( const JobsContainer_t::value_type &_node )
{ 
  //   mutex.lock();
  m_container.erase( remove( m_container.begin(), m_container.end(), _node.second.get() ),
		     m_container.end() );
  m_cur_ids.erase( _node.first );
  m_curinfo.erase( _node.first );

  emit removeJob( _node.second );
  // TODO: for Qt 4.3 use Qt::BlockingQueuedConnection
  // so-far we must support Qt 4.2, I therefore use this duty trick
  //g_signalIsPosted.wait(&mutex);
  //mutex.unlock();
}

void CJobsContainer::_updateJobInfo( const JobsContainer_t::value_type &_node )
{
    // checking status of the given job
    SJobInfo *info = _node.second.get();
    const CLsfMng::EJobStatus_t newStatus = m_lsfsubmitter->getLSF().jobStatus(info->m_id);
    if ( info->m_status == newStatus )
        return;

    info->m_status = newStatus;
    info->m_strStatus = m_lsfsubmitter->getLSF().jobStatusString(newStatus);
    emit jobChanged( info );
}
