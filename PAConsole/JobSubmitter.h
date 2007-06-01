/************************************************************************/
/**
 * @file JobSubmitter.h
 * @brief A thread-class which submits Grid jobs and reports a progress status 
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-06-01
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef JOBSUBMITTER_H_
#define JOBSUBMITTER_H_

// Qt
#include <QThread>

class CJobSubmitter: public QThread
{
        Q_OBJECT

    public:
        CJobSubmitter( QObject *parent ):
                QThread(parent),
                m_JobsCount(0)
        {}
        ~CJobSubmitter()
        {
            if ( isRunning() )
                terminate();
        }

    public:
        void set_jobs_count( int _Count )
        {
            m_JobsCount = _Count;
        }

    signals:
        void changeProgress( int _Val);

    protected:
        void run()
        {
            emit changeProgress( 0 );
            for ( size_t i = 0; i < m_JobsCount; ++i )
            {
                emit changeProgress( i * 100 / m_JobsCount );
                sleep(3);
            }
            emit changeProgress( 100 );
        }

    private:
        size_t m_JobsCount;
};

#endif /*JOBSUBMITTER_H_*/
