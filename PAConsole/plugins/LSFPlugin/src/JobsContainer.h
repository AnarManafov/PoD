/************************************************************************/
/**
 * @file JobsContainer.h
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-06
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
#ifndef JOBSCONTAINER_H_
#define JOBSCONTAINER_H_
// LSF plug-in
#include "JobInfo.h"
// STD
#include <iterator>
// Qt
#include <QObject>

typedef std::vector<SJobInfo *> SJobInfoVec_t;

class CJobsContainer: public QThread
{
    Q_OBJECT

public:
    CJobsContainer(const CLSFJobSubmitter *_lsfsubmitter);
    virtual ~CJobsContainer();

signals:
    /**
     *  The data for a job has changed.
     */
    void jobChanged( SJobInfo *_info );
    /**
     *  This indicates we are about to add a job information in the model.
     */
    void beginAddJob( SJobInfo *_info );
    /**
     *  We have finished inserting a job.
     */
    void endAddJob();
    /**
     *  This indicates we are about to remove a job in the model.  Emit the appropriate signals.
     */
    void beginRemoveJob( SJobInfo *_info );
    /**
     *  We have finished removing a job.
     */
    void endRemoveJob();

public:
    void run();
    void update( long _update_time_ms = 0 );
    void updateNumberOfJobs();
    SJobInfoVec_t::size_type getCount() const
    {
        return m_container.size();
    }
    SJobInfo *at( SJobInfoVec_t::size_type _pos ) const
    {
        if ( _pos < 0 || _pos >= m_container.size() )
            return NULL;
        return m_container[_pos];
    }
    SJobInfoVec_t::size_type getIndex( SJobInfo *_info ) const
    {
        // TODO: This should be optimized. maybe we can add an index member in the SJobInfo or something
        SJobInfoVec_t::const_iterator iter = std::find( m_container.begin(), m_container.end(), _info );
        return std::distance( m_container.begin(), iter );
    }

private slots:
    void _updateJobsStatus();
    void _updateNumberOfJobs();

private:
    void _addJobInfo( const JobsContainer_t::value_type &_node );
    void _removeJobInfo( const JobsContainer_t::value_type &_node );
    void _updateJobInfo( const JobsContainer_t::value_type &_node );

private:
    SJobInfoVec_t m_container;
    JobsContainer_t m_curinfo;
    JobsContainer_t m_cur_ids;
    const CLSFJobSubmitter *m_lsfsubmitter;
    CJobInfo m_jobInfo;
    bool m_updateNumberOfJobs;
    unsigned long m_updateInterval;
};

#endif /* JOBSCONTAINER_H_ */
