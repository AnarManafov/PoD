/************************************************************************/
/**
 * @file
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 Anar Manafov. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/bind.hpp>
// LSF plug-in
#include "JobsContainer.h"
//=============================================================================
const size_t g_maxRetryCount = 1;
//=============================================================================
using namespace std;
using namespace pbs_plug;
//=============================================================================
CJobsContainer::CJobsContainer( CPbsJobSubmitter *_submitter ):
        m_submitter( _submitter ),
        m_updateNumberOfJobs( true ),
        m_removeAllCompletedJobs( false ),
        m_updateInterval( 0 ),
        m_countOfActiveJobs( 0 )
{
    // marshal this type
    qRegisterMetaType<size_t>( "size_t" );
}
//=============================================================================
CJobsContainer::~CJobsContainer()
{
    // stop the thread
    m_updateInterval = 0;
    m_condition.wakeAll();
    // wait infinite until worker thread is done
    wait();
}
//=============================================================================
void CJobsContainer::update( long _update_time_ms )
{
    m_updateInterval = _update_time_ms;

    if ( !isRunning() )
        start();
}
//=============================================================================
void CJobsContainer::stopUpdate()
{
    m_updateInterval = 0;
}
//=============================================================================
void CJobsContainer::run()
{
    forever
    {
        if ( 0 == m_updateInterval )
            return;

        if ( m_updateNumberOfJobs )
        {
            _updateNumberOfJobs();
            m_updateNumberOfJobs = false;
        }
        else
            _updateJobsStatus();

        m_mutex.lock();
        m_condition.wait( &m_mutex, m_updateInterval );
        m_mutex.unlock();
    }
}
//=============================================================================
void CJobsContainer::updateNumberOfJobs()
{
    // TODO: instead of this duty trick try to use Qt::BlockingQueuedConnection
    m_updateNumberOfJobs = true;
    m_condition.wakeAll();
}
//=============================================================================
void CJobsContainer::removeAllCompletedJobs()
{
    m_updateNumberOfJobs = true;
    m_removeAllCompletedJobs = true;
    m_condition.wakeAll();
}
//=============================================================================
void CJobsContainer::_updateNumberOfJobs()
{
    if ( m_removeAllCompletedJobs )
    {
        m_removeAllCompletedJobs = false;

        JobsContainer_t c( m_cur_ids );

        // Delete children first
        JobsContainer_t::const_iterator iter = c.begin();
        JobsContainer_t::const_iterator iter_end = c.end();
        for ( ; iter != iter_end; ++iter )
        {
            // TODO: the way we detect parents must be moved to a separated method
            // in order to avoid of duplication
            if ( iter->first.find( '[' ) != string::npos && iter->second->m_completed )
                _removeJobInfo( *iter, false );
        }
        // Delete parents
        iter = c.begin();
        iter_end = c.end();
        for ( ; iter != iter_end; ++iter )
        {
            if ( iter->first.find( '[' ) == string::npos && iter->second->m_completed )
            {
                m_submitter->removeJob( iter->second->m_id, false );
                _removeJobInfo( *iter, true );
            }

        }
        return;
    }

    JobsContainer_t newinfo;
    m_jobInfo.update( m_submitter->getParentJobsList(), &newinfo );

    size_t count = _markAllCompletedJobs( &newinfo, false );

    if ( count != m_countOfActiveJobs )
    {
        m_countOfActiveJobs = count;
        emit numberOfActiveJobsChanged( m_countOfActiveJobs );
    }

    // adding all jobs for the first time
    if ( m_cur_ids.empty() )
    {
        for_each( newinfo.begin(), newinfo.end(),
                  boost::bind( &CJobsContainer::_addJobInfo, this, _1 ) );
        return;
    }

    // Checking jobs for removal
    JobsContainer_t tmp;
    set_difference( m_cur_ids.begin(), m_cur_ids.end(),
                    newinfo.begin(), newinfo.end(),
                    inserter( tmp, tmp.begin() ) );

    // Delete children first
    JobsContainer_t::const_iterator iter = tmp.begin();
    JobsContainer_t::const_iterator iter_end = tmp.end();
    for ( ; iter != iter_end; ++iter )
    {
        if ( iter->first.find( '[' ) != string::npos )
            _removeJobInfo( *iter, false );
    }
    // Delete parents
    iter = tmp.begin();
    iter_end = tmp.end();
    for ( ; iter != iter_end; ++iter )
    {
        if ( iter->first.find( '[' ) == string::npos )
            _removeJobInfo( *iter, true );
    }

    // Checking for newly added jobs
    tmp.clear();
    set_difference( newinfo.begin(), newinfo.end(),
                    m_cur_ids.begin(), m_cur_ids.end(),
                    inserter( tmp, tmp.begin() ) );
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_addJobInfo, this, _1 ) );
}
//=============================================================================
void CJobsContainer::_updateJobsStatus()
{
    // TODO: for parent jobs just print a statistics information (X - pending; Y - run; ...)
    size_t count = _markAllCompletedJobs( &m_cur_ids );
    if ( count != m_countOfActiveJobs )
    {
        m_countOfActiveJobs = count;
        emit numberOfActiveJobsChanged( m_countOfActiveJobs );
    }
}
//=============================================================================
void CJobsContainer::_addJobInfo( const JobsContainer_t::value_type &_node )
{
    SJobInfo *info = _node.second;

    pair<JobsContainer_t::iterator, bool> res =  m_cur_ids.insert( JobsContainer_t::value_type( info->m_strID, info ) );
    if ( res.second && NULL == info->parent() )
    {
        emit addJob( info );
    }
}
//=============================================================================
void CJobsContainer::_removeJobInfo( const JobsContainer_t::value_type &_node, bool _emitUpdate )
{
    m_cur_ids.erase( _node.first );

    if ( _emitUpdate )
        emit removeJob( _node.second );
}
//=============================================================================
size_t CJobsContainer::_markAllCompletedJobs( JobsContainer_t * _container, bool _emitUpdate )
{
    size_t run_jobs( 0 );

    // if all jobs are marked already as completed, we don't need to ask PBS
    JobsContainer_t::const_iterator iter = _container->begin();
    JobsContainer_t::const_iterator iter_end = _container->end();
    bool need_request( false );
    for ( ; iter != iter_end; ++iter )
    {
        if ( !iter->second->m_completed )
            need_request = true;
    }

    if ( !need_request )
        return run_jobs;

    // Getting a status for all available jobs

    // FIXME: Be advised, if job's status is asked too fast (immediately after submission),
    // than it could be the case that a PBS server is too slow to register the job
    // and it will not return its status.
    // We need to have a counter before marking a job as completed
    CPbsMng::jobInfoContainer_t all_available_jobs;
    try
    {
        m_submitter->jobStatusAllJobs( &all_available_jobs );
    }
    catch ( const exception &_e )
    {
        // TODO: show message
    }

    iter = _container->begin();
    iter_end = _container->end();
    for ( ; iter != iter_end; ++iter )
    {
        // Bad ID or a root item
        if ( iter->first.empty() )
            continue;

        if ( iter->second->m_completed )
            continue;

        CPbsMng::jobInfoContainer_t::const_iterator found = all_available_jobs.find( iter->second->m_id );
        if ( all_available_jobs.end() == found )
        {
            // if the parent, we don't write any status so-far,
            // in PBS plug-in, a parent job is just a fake and can't have a
            // status.
            // FIXME: implement a status lagorthm for parent jobs (some summary of
            // status of children)
            if ( CPbsMng::isParentID( iter->first ) )
            {
                iter->second->m_completed = true;
                iter->second->m_status = "completed";
                iter->second->m_strStatus = "(expand to see the status)";
                continue;
            }

            ++iter->second->m_tryCount;
            if ( iter->second->m_tryCount > g_maxRetryCount )
            {
                // set job to completed
                iter->second->m_completed = true;
                iter->second->m_status = "completed";
                iter->second->m_strStatus = "completed";
            }
            else
            {
                iter->second->m_strStatus = "not known yet...";
            }

        }
        else
        {
            // calculate a number of expected PoD workers.
            // We exclude "parent" job ids
            if ( !CPbsMng::isParentID( iter->first ) )
                ++run_jobs;

            if ( iter->second->m_status == found->second.m_status )
                continue;

            iter->second->m_completed = CPbsMng::isJobComplete( found->second.m_status );
            if ( iter->second->m_completed )
                --run_jobs;

            iter->second->m_status = found->second.m_status;
            iter->second->m_strStatus = CPbsMng::jobStatusToString( found->second.m_status );
        }
        if ( _emitUpdate )
            emit jobChanged( iter->second );
    }

    return run_jobs;
}
