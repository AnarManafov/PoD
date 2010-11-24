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
//const size_t g_maxRetryCount = 1;
//=============================================================================
using namespace std;
using namespace oge_plug;
//=============================================================================
CJobsContainer::CJobsContainer( COgeJobSubmitter *_submitter ):
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

    if( !isRunning() )
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
        if( 0 == m_updateInterval )
            return;

        if( m_updateNumberOfJobs )
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
    qDebug( "CJobsContainer::_updateNumberOfJobs" );

    if( m_removeAllCompletedJobs )
    {
        qDebug( "CJobsContainer::_updateNumberOfJobs: remove all completed" );
        m_removeAllCompletedJobs = false;

        JobsContainer_t c( m_cur_ids );

        JobsContainer_t::const_iterator iter = c.begin();
        JobsContainer_t::const_iterator iter_end = c.end();
        for( ; iter != iter_end; ++iter )
        {
            if( !COgeMng::isParentID( iter->first ) )
                continue;

            if( iter->second->allChildrenCompleted() )
            {
                // remove children
                jobs_children_t::const_iterator c = iter->second->m_children.begin();
                jobs_children_t::const_iterator c_end = iter->second->m_children.end();
                for( ; c != c_end; ++c )
                {
                    SJobInfo *inf = *c;
                    _removeJobInfo( JobsContainer_t::value_type( inf->m_id, inf ), false );
                }
                // remove the parent
                m_submitter->removeJob( iter->second->m_id, false );
                _removeJobInfo( *iter, true );
            }
        }
        return;
    }

    JobsContainer_t newinfo;
    m_jobInfo.update( m_submitter->getParentJobsList(), &newinfo );

    size_t count = _markAllCompletedJobs( &newinfo, false );

    if( count != m_countOfActiveJobs )
    {
        m_countOfActiveJobs = count;
        emit numberOfActiveJobsChanged( m_countOfActiveJobs );
    }

    // adding all jobs for the first time
    if( m_cur_ids.empty() )
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
    for( ; iter != iter_end; ++iter )
    {
        if( !COgeMng::isParentID( iter->first ) )
            _removeJobInfo( *iter, false );
    }
    // Delete parents
    iter = tmp.begin();
    iter_end = tmp.end();
    for( ; iter != iter_end; ++iter )
    {
        if( COgeMng::isParentID( iter->first ) )
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
    if( count != m_countOfActiveJobs )
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
    if( res.second && NULL == info->parent() )
    {
        info->m_strStatus = "(expand to see the status)";
        emit addJob( info );
    }
}
//=============================================================================
void CJobsContainer::_removeJobInfo( const JobsContainer_t::value_type &_node, bool _emitUpdate )
{
    qDebug( "CJobsContainer::_removeJobInfo: remove id=%s", _node.first.c_str() );
    m_cur_ids.erase( _node.first );

    if( _emitUpdate )
        emit removeJob( _node.second );
}
//=============================================================================
size_t CJobsContainer::_markAllCompletedJobs( JobsContainer_t * _container, bool _emitUpdate )
{
    qDebug( "CJobsContainer::_markAllCompletedJobs" );
    size_t run_jobs( 0 );

    // if all jobs are marked already as completed, we don't need to ask OGE
    JobsContainer_t::const_iterator iter = _container->begin();
    JobsContainer_t::const_iterator iter_end = _container->end();
    bool need_request( false );
    for( ; iter != iter_end; ++iter )
    {
        if( COgeMng::isParentID( iter->first ) )
            continue;

        if( !iter->second->m_completed )
        {
            need_request = true;
            break;
        }
    }

    if( !need_request )
        return run_jobs;

    iter = _container->begin();
    iter_end = _container->end();
    for( ; iter != iter_end; ++iter )
    {
        qDebug( "CJobsContainer::_markAllCompletedJobs: job=%s",
                iter->first.c_str() );
        // Bad ID or a root item
        if( iter->first.empty() )
            continue;

        if( iter->second->m_completed )
            continue;

        // if the parent, we don't write any status so-far,
        // in OGE plug-in, a parent job is just a fake and can't have a
        // status.
        if( COgeMng::isParentID( iter->first ) )
        {
            qDebug( "CJobsContainer::_markAllCompletedJobs: %s is a parent job",
                    iter->first.c_str() );
            //iter->second->m_strStatus = "(expand to see the status)";
        }
        else
        {
            int status = m_submitter->jobStatus( iter->first );
            if( m_submitter->isJobComplete( status ) )
            {
                qDebug( "CJobsContainer::_markAllCompletedJobs: %s is completed",
                        iter->first.c_str() );
                // set job to completed
                iter->second->m_completed = true;
            }
            else
            {
                qDebug( "CJobsContainer::_markAllCompletedJobs: %s is running",
                        iter->first.c_str() );
                ++run_jobs;
            }
            iter->second->m_status = m_submitter->jobStatusAsString( status );
            iter->second->m_strStatus = iter->second->m_status;
        }

        if( _emitUpdate )
            emit jobChanged( iter->second );
    }

    qDebug( "CJobsContainer::_markAllCompletedJobs: running jobs=%d", (int)run_jobs );
    return run_jobs;
}
