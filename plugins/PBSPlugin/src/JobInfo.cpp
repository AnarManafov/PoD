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
#include "JobInfo.h"
//=============================================================================
using namespace std;
using namespace pbs_plug;
//=============================================================================
CJobInfo::CJobInfo( const CPbsMng &_pbs ): m_pbs( _pbs )
{
}
//=============================================================================
CJobInfo::~CJobInfo()
{
}
//=============================================================================
void CJobInfo::update( const CPbsJobSubmitter::jobslist_t &_Jobs,
                       JobsContainer_t *_Container )
{
    // TODO: !!! the entire algorithm MUST be revised - since it is very inefficient !!!
    // it regenerates every time the entire list of jobs from very beginning

    // TODO: better error handling in this function
    JobsContainer_t copyContainer( m_Container );
    m_Container.clear();

    CPbsJobSubmitter::jobslist_t::const_iterator iter = _Jobs.begin();
    CPbsJobSubmitter::jobslist_t::const_iterator iter_end = _Jobs.end();
    for( ; iter != iter_end; ++iter )
    {
        // checking the old status
        JobsContainer_t::iterator found = copyContainer.find( iter->first );
        // don't need to update this job
        if( copyContainer.end() != found )
        {
            m_Container.insert( *found );
            if( _Container )
            {
                // adding child jobs to the output container
                // TODO: do it in getInfo method
                for( int i = 0; i < found->second->m_children.size(); ++i )
                    _Container->insert( JobsContainer_t::value_type( found->second->m_children[i]->m_strID,
                                                                     found->second->m_children[i] ) );
            }
            continue;
        }

        // Creating new item
        SJobInfo *info( new SJobInfo( iter->first ) );

        // adding children
        for( size_t i = CPbsMng::jobArrayStartIdx(); i < iter->second; ++i )
        {

            addChildItem( CPbsMng::generateArrayJobID( iter->first, i ),
                          info );
        }

        // adding parent job
        m_Container.insert( JobsContainer_t::value_type( info->m_strID, info ) );

        if( _Container )
        {
            // adding child jobs to the output container
            // TODO: do it in getInfo method
            for( int i = 0; i < info->m_children.size(); ++i )
                _Container->insert( JobsContainer_t::value_type( info->m_children[i]->m_strID,
                                                                 info->m_children[i] ) );
        }
    }

    // a result set
    if( _Container )
        copy( m_Container.begin(), m_Container.end(),
              inserter( *_Container, _Container->begin() ) );
}
//=============================================================================
void CJobInfo::addChildItem( const CPbsMng::jobID_t &_JobID, SJobInfo *_parent ) const
{
    SJobInfo *info( new SJobInfo( _JobID, _parent ) );
    _parent->addChild( info );
}
