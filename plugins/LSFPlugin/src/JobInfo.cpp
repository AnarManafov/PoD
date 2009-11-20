/************************************************************************/
/**
 * @file JobInfo.cpp
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-09
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/bind.hpp>
// LSF plug-in
#include "JobInfo.h"
//=============================================================================
using namespace std;
//=============================================================================
CJobInfo::CJobInfo( const CLsfMng &_lsf ):
        m_lsf( _lsf )
{
}
//=============================================================================
CJobInfo::~CJobInfo()
{
}
//=============================================================================
void CJobInfo::update( const CLSFJobSubmitter::jobslist_t &_Jobs, JobsContainer_t *_Container )
{
    // TODO: !!! the entire algorithm MUST be revised - since it is very inefficient !!!
    // it regenerates every time the entire list of jobs from very beginning

    // TODO: better error handling in this function
    JobsContainer_t copyContainer( m_Container );
    m_Container.clear();

    CLSFJobSubmitter::jobslist_t::const_iterator iter = _Jobs.begin();
    CLSFJobSubmitter::jobslist_t::const_iterator iter_end = _Jobs.end();
    for ( ; iter != iter_end; ++iter )
    {
        std::ostringstream str;
        // TODO: LsfMng should have methods to return string representations of jobID
        str << LSB_ARRAY_JOBID( *iter );

        // checking the old status
        JobsContainer_t::iterator found = copyContainer.find( str.str() );
        // don't need to update this job
        if ( copyContainer.end() != found )
        {
            m_Container.insert( *found );
            continue;
        }

        SJobInfoPTR_t info( new SJobInfo() );
        info->m_id = *iter;
        info->m_strID = str.str();

        try
        {
            CLsfMng::IDContainer_t children;
            m_lsf.getChildren( *iter, &children );
            std::for_each( children.begin(), children.end(),
                           boost::bind( boost::mem_fn( &CJobInfo::addChildItem ), this, _1, info ) );
        }
        catch ( const std::exception &_e )
            {}
        // adding parent job
        m_Container.insert( JobsContainer_t::value_type( info->m_strID, info ) );


        if ( _Container )
        {
            // adding child jobs to the output container
            // TODO: do it in getInfo method
            jobs_children_t::const_iterator iter = info->m_children.begin();
            jobs_children_t::const_iterator iter_end = info->m_children.end();
            for ( ; iter != iter_end; ++iter )
            {
                _Container->insert( JobsContainer_t::value_type( iter->get()->m_strID, *iter ) );
            }
        }
    }

    // a result set
    getInfo( _Container );
}
//=============================================================================
void CJobInfo::addChildItem( lsf_jobid_t _JobID, SJobInfoPTR_t _parent )
{
    std::ostringstream str;
    str << LSB_ARRAY_JOBID( _JobID ) << "[" << LSB_ARRAY_IDX( _JobID ) << "]";
    SJobInfoPTR_t info( new SJobInfo() );
    info->m_id = _JobID;
    info->m_strID = str.str();

    info->m_parent = _parent.get();

    _parent->addChild( info );
}
//=============================================================================
void CJobInfo::getInfo( JobsContainer_t *_Container ) const
{
    if ( _Container )
        copy( m_Container.begin(), m_Container.end(),
              inserter( *_Container, _Container->begin() ) );
}
