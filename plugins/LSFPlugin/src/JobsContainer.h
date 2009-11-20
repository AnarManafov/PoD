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
#include <QtCore>
//=============================================================================
class CJobsContainer: public QThread
{
        Q_OBJECT

    public:
        CJobsContainer( const CLSFJobSubmitter *_lsfsubmitter );
        virtual ~CJobsContainer();

    signals:
        /**
         *  The data for a job has changed.
         */
        void jobChanged( SJobInfo *_info );
        /**
         *  This indicates we are about to add a job information in the model.
         */
        void addJob( const SJobInfoPTR_t &_info );
        /**
         *  This indicates we are about to remove a job in the model.  Emit the appropriate signals.
         */
        void removeJob( const SJobInfoPTR_t &_info );
        void numberOfActiveJobsChanged( size_t _count );

    public:
        void run();
        void update( long _update_time_ms = 0 );
        void stopUpdate();
        void updateNumberOfJobs();

    private slots:
        void _updateJobsStatus();
        void _updateNumberOfJobs();

    private:
        void _addJobInfo( const JobsContainer_t::value_type &_node );
        void _removeJobInfo( const JobsContainer_t::value_type &_node );
        size_t _markAllCompletedJobs( JobsContainer_t * _container );

    private:
        JobsContainer_t m_curinfo;
        JobsContainer_t m_cur_ids;
        const CLSFJobSubmitter *m_lsfsubmitter;
        CJobInfo m_jobInfo;
        bool m_updateNumberOfJobs;
        unsigned long m_updateInterval;
        QWaitCondition m_condition;
        QMutex m_mutex;
        size_t m_countOfActiveJobs;
};

#endif /* JOBSCONTAINER_H_ */
