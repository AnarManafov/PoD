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

using namespace std;

CJobInfo::CJobInfo(const CLsfMng &_lsf):
        m_lsf(_lsf)
{
}

CJobInfo::~CJobInfo()
{
}

void CJobInfo::update( const CLSFJobSubmitter::jobslist_t &_Jobs, JobsContainer_t *_Container )
{
    // TODO: better error handling in this function
    m_Container.clear();

    CLSFJobSubmitter::jobslist_t::const_iterator iter = _Jobs.begin();
    CLSFJobSubmitter::jobslist_t::const_iterator iter_end = _Jobs.end();
    for (; iter != iter_end; ++iter)
    {
        SJobInfo *info = new SJobInfo();
        info->m_id = *iter;
        std::ostringstream str;
        // TODO: LsfMng should have methods to return string representations of jobID
        str << LSB_ARRAY_JOBID(*iter);
        info->m_strID = str.str();
        info->m_status = m_lsf.jobStatus(*iter);
        info->m_strStatus = m_lsf.jobStatusString(*iter);
        try
        {
            CLsfMng::IDContainer_t children;
            m_lsf.getChildren( *iter, &children );
            std::for_each( children.begin(), children.end(),
                           boost::bind( boost::mem_fn( &CJobInfo::addChildItem ), this, _1, info ) );
        }
        catch ( const std::exception &_e )
            {}

        m_Container.insert( JobsContainer_t::value_type( *iter, info ) );
    }

    // a result set
    getInfo( _Container );
}

void CJobInfo::addChildItem( lsf_jobid_t _JobID, SJobInfo *_parent )
{
    std::ostringstream str;
    str << LSB_ARRAY_JOBID(_JobID) << "[" << LSB_ARRAY_IDX(_JobID) << "]";
    SJobInfo *info = new SJobInfo();
    info->m_id = LSB_ARRAY_JOBID(_JobID);
    info->m_strID = str.str();
    info->m_status = m_lsf.jobStatus(_JobID);
    info->m_strStatus = m_lsf.jobStatusString(_JobID);
    info->m_parent = _parent;
    _parent->addChild( info );
}

void CJobInfo::getInfo( JobsContainer_t *_Container )
{
    if ( _Container )
        copy( m_Container.begin(), m_Container.end(),
              inserter( *_Container, _Container->begin() ) );
}
