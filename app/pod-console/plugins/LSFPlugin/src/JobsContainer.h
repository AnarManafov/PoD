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
// Qt
#include <QObject>
#include <QtCore>
//=============================================================================
class CJobsContainer: public QThread
{
        Q_OBJECT

    public:
        CJobsContainer( CLSFJobSubmitter *_lsfsubmitter );
        virtual ~CJobsContainer();

    signals:
        /**
         *  The data for a job has changed.
         */
        void jobChanged( SJobInfo *_info );
        /**
         *  This indicates we are about to add a job information in the model.
         */
        void addJob( SJobInfo *_info );
        /**
         *  This indicates we are about to remove a job in the model.  Emit the appropriate signals.
         */
        void removeJob( SJobInfo *_info );
        void numberOfActiveJobsChanged( size_t _count );

    public:
        void run();
        void update( long _update_time_ms = 0 );
        void stopUpdate();
        void updateNumberOfJobs();
        void removeAllCompletedJobs();

    private slots:
        void _updateJobsStatus();
        void _updateNumberOfJobs();

    private:
        void _addJobInfo( const JobsContainer_t::value_type &_node );
        void _removeJobInfo( const JobsContainer_t::value_type &_node, bool _emitUpdate );
        size_t _markAllCompletedJobs( JobsContainer_t * _container, bool _emitUpdate = true );

    private:
        JobsContainer_t m_cur_ids;
        CLSFJobSubmitter *m_lsfsubmitter;
        CJobInfo m_jobInfo;
        bool m_updateNumberOfJobs;
        bool m_removeAllCompletedJobs;
        unsigned long m_updateInterval;
        QWaitCondition m_condition;
        QMutex m_mutex;
        size_t m_countOfActiveJobs;
};

#endif /* JOBSCONTAINER_H_ */
