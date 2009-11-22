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
//=============================================================================
using namespace std;
//=============================================================================
CJobsContainer::CJobsContainer( const CLSFJobSubmitter *_lsfsubmitter ):
        m_lsfsubmitter( _lsfsubmitter ),
        m_jobInfo( _lsfsubmitter->getLSF() ),
        m_updateNumberOfJobs( true ),
        m_updateInterval( 0 ),
        m_countOfActiveJobs( 0 )
{
    // we need to register SJobInfoPTR_t, so that Qt will be able to
    // marshal this type

  //  qRegisterMetaType<SJobInfoPTR_t>( "SJobInfoPTR_t" );
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
    // TODO: for Qt 4.3 use Qt::BlockingQueuedConnection
    // so-far we must support Qt 4.2, I therefore use this duty trick
    m_updateNumberOfJobs = true;
    m_condition.wakeAll();
}
//=============================================================================
void CJobsContainer::_updateNumberOfJobs()
{
    JobsContainer_t newinfo;
    m_jobInfo.update( m_lsfsubmitter->getParentJobsList(), &newinfo );

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
        if ( NULL != iter->second->m_parent && 0 != iter->second->m_parent->m_id )
            _removeJobInfo( *iter, false );
    }
    // Delete parents
    iter = tmp.begin();
    for ( ; iter != iter_end; ++iter )
    {
        if ( NULL == iter->second->m_parent || 0 == iter->second->m_parent->m_id )
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
    if ( res.second )
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

    // if there all jobs a marked already as completed, we don't need to ask LSF
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

    // Getting a list of unfinished LSF jobs
    CLsfMng::IDContainerOrdered_t unfinished;
    run_jobs = m_lsfsubmitter->getLSF().getAllUnfinishedJobs( &unfinished );

    run_jobs = unfinished.size();

    iter = _container->begin();
    iter_end = _container->end();
    for ( ; iter != iter_end; ++iter )
    {
        if ( iter->second->m_completed )
            continue;

        CLsfMng::IDContainerOrdered_t::const_iterator found = unfinished.find( iter->second->m_id );
        if ( unfinished.end() == found )
        {
            // set job to completed
            iter->second->m_completed = true;
            iter->second->m_status = JOB_STAT_UNKWN;
            iter->second->m_strStatus = "completed";
        }
        else
        {
            cout << LSB_ARRAY_JOBID( iter->second->m_id ) << "[" << LSB_ARRAY_IDX( iter->second->m_id ) << "]: " << found->second << endl;
            if ( iter->second->m_status == found->second )
                continue;

            iter->second->m_completed = false;
            iter->second->m_status = found->second;
            iter->second->m_strStatus = CLsfMng::jobStatusString( found->second );
        }
        if ( _emitUpdate )
            emit jobChanged( iter->second );
    }

    return run_jobs;
}
