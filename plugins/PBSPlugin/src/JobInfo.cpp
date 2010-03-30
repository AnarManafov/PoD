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
//CJobInfo::CJobInfo( const CLsfMng &_lsf ): m_lsf( _lsf )
//{
//}
////=============================================================================
//CJobInfo::~CJobInfo()
//{
//}
////=============================================================================
//void CJobInfo::update( const CLSFJobSubmitter::jobslist_t &_Jobs, JobsContainer_t *_Container )
//{
//    // TODO: !!! the entire algorithm MUST be revised - since it is very inefficient !!!
//    // it regenerates every time the entire list of jobs from very beginning
//
//    // TODO: better error handling in this function
//    JobsContainer_t copyContainer( m_Container );
//    m_Container.clear();
//
//    CLSFJobSubmitter::jobslist_t::const_iterator iter = _Jobs.begin();
//    CLSFJobSubmitter::jobslist_t::const_iterator iter_end = _Jobs.end();
//    for ( ; iter != iter_end; ++iter )
//    {
//        std::ostringstream str;
//        str << LSB_ARRAY_JOBID( *iter );
//        // checking the old status
//        JobsContainer_t::iterator found = copyContainer.find( str.str() );
//        // don't need to update this job
//        if ( copyContainer.end() != found )
//        {
//            m_Container.insert( *found );
//            if ( _Container )
//            {
//                // adding child jobs to the output container
//                // TODO: do it in getInfo method
//                for ( int i = 0; i < found->second->m_children.size(); ++i )
//                    _Container->insert( JobsContainer_t::value_type( found->second->m_children[i]->m_strID, found->second->m_children[i] ) );
//            }
//            continue;
//        }
//
//        // Creating new item
//        SJobInfo *info( new SJobInfo( *iter ) );
//
//        try
//        {
//            // getting a list of children
//            CLsfMng::IDContainer_t children;
//            m_lsf.getChildren( *iter, &children );
//            std::for_each( children.begin(), children.end(),
//                           boost::bind( boost::mem_fn( &CJobInfo::addChildItem ), this, _1, info ) );
//        }
//        catch ( const std::exception &_e )
//            {}
//        // adding parent job
//        m_Container.insert( JobsContainer_t::value_type( info->m_strID, info ) );
//
//        if ( _Container )
//        {
//            // adding child jobs to the output container
//            // TODO: do it in getInfo method
//            for ( int i = 0; i < info->m_children.size(); ++i )
//                _Container->insert( JobsContainer_t::value_type( info->m_children[i]->m_strID, info->m_children[i] ) );
//        }
//    }
//
//    // a result set
//    if ( _Container )
//        copy( m_Container.begin(), m_Container.end(),
//              inserter( *_Container, _Container->begin() ) );
//}
////=============================================================================
//void CJobInfo::addChildItem( lsf_jobid_t _JobID, SJobInfo *_parent ) const
//{
//    SJobInfo *info( new SJobInfo( _JobID, _parent ) );
//    _parent->addChild( info );
//}
