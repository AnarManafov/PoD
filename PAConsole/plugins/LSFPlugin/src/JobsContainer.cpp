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
// vnetstat
#include "JobsContainer.h"

CJobsContainer::CJobsContainer( const CLSFJobSubmitter *_lsfsubmitter):m_lsfsubmitter(_lsfsubmitter)
{
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( _update() ) );
}

CJobsContainer::~CJobsContainer()
{
}

void CJobsContainer::update( long _update_time_ms )
{
    // Let view immediately get an update
    _update();
    // Resetting a timer
    m_timer->start( _update_time_ms );
}

void CJobsContainer::_update()
{
    JobsContainer_t newinfo;
    CJobInfo job_info( m_lsfsubmitter->getLSF() );
    job_info.update( m_lsfsubmitter->getActiveJobList(), &newinfo );

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
                    inserter( tmp, tmp.begin() ) );
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_removeJobInfo, this, _1 ) );

    // Checking all jobs for update
    tmp.clear();
    set_intersection( newinfo.begin(), newinfo.end(),
                      m_cur_ids.begin(), m_cur_ids.end(),
                      inserter( tmp, tmp.begin() ) );
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_updateJobInfo, this, _1 ) );


    // Checking for newly added jobs
    tmp.clear();
    set_difference( newinfo.begin(), newinfo.end(),
                    m_cur_ids.begin(), m_cur_ids.end(),
                    inserter( tmp, tmp.begin() ) );
    for_each( tmp.begin(), tmp.end(),
              boost::bind( &CJobsContainer::_addJobInfo, this, _1 ) );
}

void CJobsContainer::_addJobInfo( const JobsContainer_t::value_type &_node )
{
    SJobInfoPTR_t info( _node.second );

    // parent jobs
    emit beginAddJob( info.get() );
    m_curinfo.insert( JobsContainer_t::value_type( info->m_strID, info ) );
    m_cur_ids.insert( JobsContainer_t::value_type( info->m_strID, info ) );
    m_container.push_back( info.get() );
    emit endAddJob();

    // adding children to the model
    jobs_children_t::const_iterator iter = info.get()->m_children.begin();
    jobs_children_t::const_iterator iter_end = info.get()->m_children.end();
    for(; iter != iter_end; ++iter)
    {
    	emit beginAddJob( iter->get() );
        m_curinfo.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
        m_cur_ids.insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
    	emit endAddJob();
    }
}

void CJobsContainer::_removeJobInfo( const JobsContainer_t::value_type &_node )
{
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
	JobsContainer_t::iterator found = m_curinfo.find( _node.first );
    if ( m_curinfo.end() == found )
        return; // TODO: assert here?

    if ( *( found->second.get() ) == *(_node.second) )
        return;

    *( found->second.get() ) = *(_node.second);
    emit jobChanged( found->second.get() );
}
