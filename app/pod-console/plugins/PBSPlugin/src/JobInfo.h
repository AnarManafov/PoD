/************************************************************************/
/**
 * @file PBSPlugin/src/JobInfo.h
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 Anar Manafov. All rights reserved.
*************************************************************************/
#ifndef PBS_JOBINFO_H_
#define PBS_JOBINFO_H_
//=============================================================================
// STD
#include <algorithm>
// pbs plug-in
#include "PbsJobSubmitter.h"

// TODO: release a general impelemention of JobInfo, Submitter and ItemModel
// for all plug-ins (which needs Tree View for jobs).

namespace pbs_plug
{
//=============================================================================
    struct SJobInfo;
//=============================================================================
    typedef std::map<CPbsMng::jobID_t, SJobInfo *> JobsContainer_t;
    typedef QList<SJobInfo *> jobs_children_t;
//=============================================================================
    struct SJobInfo
    {
            SJobInfo():
                m_expanded( false ),
                m_completed( false ),
                m_tryCount( 0 ),
                m_parent( NULL )
            {
            }
            SJobInfo( const CPbsMng::jobID_t &_id, SJobInfo *_parent = NULL ):
                m_id( _id ),
                m_expanded( false ),
                m_completed( false ),
                m_tryCount( 0 ),
                m_parent( _parent )
            {
                m_id = _id;
                m_strID = _id;
            }
            ~SJobInfo()
            {
                removeAllChildren();
            }
            void setParent( SJobInfo *_parent )
            {
                m_parent = _parent;
            }
            SJobInfo *parent()
            {
                return m_parent;
            }
            bool operator ==( const SJobInfo &_info )
            {
                if( m_id != _info.m_id )
                    return false;
                if( m_status != _info.m_status )
                    return false;

                return true;
            }
            int addChild( SJobInfo *_child )
            {
                m_children.push_back( _child );
                return m_children.size() - 1;
            }
            void removeAllChildren()
            {
                while( !m_children.isEmpty() )
                {
                    SJobInfo * p( m_children.takeFirst() );
                    delete p;
                }
                m_children.clear();
            }

            CPbsMng::jobID_t m_id;
            std::string m_strID;
            std::string m_status;
            std::string m_strStatus;
            bool m_expanded;
            bool m_completed; //!< if false, we don't need to monitor this job
            jobs_children_t m_children;
            // how many times a job must be checked before getting a complete status
            size_t m_tryCount;

        private:
            SJobInfo *m_parent; //!< parent of this job or NULL
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
            CJobInfo();
            virtual ~CJobInfo();

        public:
            void update( const CPbsJobSubmitter::jobslist_t &_Jobs,
                         JobsContainer_t *_Container = NULL );

        private:
            void addChildItem( const CPbsMng::jobID_t &_JobID, SJobInfo *_parent ) const;

        private:
            JobsContainer_t m_Container;
    };
}
#endif /* PBS_JOBINFO_H_ */
