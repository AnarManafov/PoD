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

// TODO: remove this include (was used for debug only)
#include <iostream>

using namespace std;

CJobsContainer::CJobsContainer( const CLSFJobSubmitter *_lsfsubmitter):
        m_lsfsubmitter(_lsfsubmitter),
        m_jobInfo(_lsfsubmitter->getLSF()),
        m_updateNumberOfJobs(false),
        m_updateInterval(0)
{
    //  m_timer = new QTimer( this );
    //  connect( m_timer, SIGNAL( timeout() ), this, SLOT( _update() ) );

    _updateNumberOfJobs();
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
    // Resetting a timer
    //m_timer->start( _update_time_ms );

    m_updateInterval = _update_time_ms;

    if ( !isRunning() )
        start();
}

//void CJobsContainer::_update()
//{
//    if ( isRunning() )
//        return;
//
//    run();
//}

void CJobsContainer::run()
{
    while (true)
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
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_removeJobInfo, this, _1 ) );

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
    SJobInfoPTR_t info( _node.second );

    pair<JobsContainer_t::iterator, bool> res =  m_cur_ids.insert( JobsContainer_t::value_type( info->m_strID, info ) );

    if ( res.second )
    {
        cout << "add: " << _node.second->m_strID << endl;

        // parent jobs
        emit beginAddJob( info.get() );
        m_curinfo.insert( JobsContainer_t::value_type( info->m_strID, info ) );
        m_container.push_back( info.get() );
        emit endAddJob();
    }

    // adding children to the model
    jobs_children_t::const_iterator iter = info.get()->m_children.begin();
    jobs_children_t::const_iterator iter_end = info.get()->m_children.end();
    for (; iter != iter_end; ++iter)
    {
        res = m_cur_ids.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
        if ( res.second )
        {
            cout << "add: " << iter->get()->m_strID << endl;
            emit beginAddJob( iter->get() );
            m_curinfo.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
            emit endAddJob();
        }
    }
}

void CJobsContainer::_removeJobInfo( const JobsContainer_t::value_type &_node )
{
    cout << "remove: " << _node.second->m_strID << endl;
    JobsContainer_t::iterator found = m_curinfo.find( _node.first );
    if ( m_curinfo.end() == found )
        return; // TODO: assert here?

    emit beginRemoveJob( found->second.get() );
    m_cur_ids.erase( found->first );
    m_curinfo.erase( found );
    m_container.erase( remove( m_container.begin(), m_container.end(), found->second.get() ),
                       m_container.end() );
    emit endRemoveJob();
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
    cout << "update: " << info->m_strID << "; Status: " << info->m_strStatus << endl;
    emit jobChanged( info );
}
