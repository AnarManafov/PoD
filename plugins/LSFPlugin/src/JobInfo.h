/************************************************************************/
/**
 * @file JobInfo.h
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-06
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
#ifndef JOBINFO_H_
#define JOBINFO_H_
//=============================================================================
// BOOST
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
// STD
#include <algorithm>
// lsf plug-in
#include "LSFJobSubmitter.h"
// STD
#include <iostream>
//=============================================================================
struct SJobInfo;
//=============================================================================
typedef std::map<std::string, SJobInfo *> JobsContainer_t;
typedef QList<SJobInfo *> jobs_children_t;
//=============================================================================
struct SJobInfo
{
    SJobInfo( lsf_jobid_t _id, SJobInfo *_parent = NULL ):
            m_id( _id ),
            m_status( JOB_STAT_UNKWN ),
            m_parent( _parent ),
            m_expanded( false ),
            m_completed( false )
    {
        std::ostringstream str;
        if ( NULL != m_parent )
            str << LSB_ARRAY_JOBID( _id ) << "[" << LSB_ARRAY_IDX( _id ) << "]";
        else
            str << LSB_ARRAY_JOBID( _id );

        m_id = _id;
        m_strID = str.str();

        std::cout << "CREATE " << m_strID << std::endl;
    }
    ~SJobInfo()
    {
        // TODO: REMOVE DEBUG
        std::cout << "DELETE " << m_strID << std::endl;
        for ( int i = 0; i < m_children.size(); ++i )
            delete m_children[i];
    }
    bool operator ==( const SJobInfo &_info )
    {
        if ( m_strID != _info.m_strID )
            return false;
        if ( m_status != _info.m_status )
            return false;

        return true;
    }
    int addChild( SJobInfo *_child )
    {
        m_children.push_back( _child );
        return m_children.size() - 1;
    }

    lsf_jobid_t m_id;
    std::string m_strID;
    int m_status;
    std::string m_strStatus;
    jobs_children_t m_children;
    SJobInfo *m_parent; //!< parent of this job or NULL
    bool m_expanded;
    bool m_completed; //!< if false, we don't need to monitor this job
};
//=============================================================================
/**
 *
 *  this comparison operator is required to use the container with generic algorithms
 *
 */
inline bool operator <( const JobsContainer_t::value_type &_v1,
                        const JobsContainer_t::value_type &_v2 )
{
    return _v1.first < _v2.first;
}
//=============================================================================
class CJobInfo
{
    public:
        CJobInfo( const CLsfMng &_lsf );
        virtual ~CJobInfo();

    public:
        void update( const CLSFJobSubmitter::jobslist_t &_Jobs, JobsContainer_t *_Container = NULL );

    private:
        void addChildItem( lsf_jobid_t _JobID, SJobInfo *_parent ) const;

    private:
        JobsContainer_t m_Container;
        const CLsfMng &m_lsf;
};

#endif /* JOBINFO_H_ */
