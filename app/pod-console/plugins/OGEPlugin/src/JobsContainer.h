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
#ifndef PBSJOBSCONTAINER_H_
#define PBSJOBSCONTAINER_H_
// pbs plug-in
#include "JobInfo.h"
// Qt
#include <QObject>
#include <QtCore>

namespace pbs_plug
{
//=============================================================================
    class CJobsContainer: public QThread
    {
            Q_OBJECT

        public:
            CJobsContainer( CPbsJobSubmitter *_submitter );
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
            CPbsJobSubmitter *m_submitter;
            CJobInfo m_jobInfo;
            bool m_updateNumberOfJobs;
            bool m_removeAllCompletedJobs;
            unsigned long m_updateInterval;
            QWaitCondition m_condition;
            QMutex m_mutex;
            size_t m_countOfActiveJobs;
    };
}

#endif /* PBSJOBSCONTAINER_H_ */
