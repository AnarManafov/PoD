/************************************************************************/
/**
 * @file JobSubmitter.h
 * @brief A thread-class which submits Grid jobs and reports a progress status
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:   $LastChangedRevision: 851 $
        created by:          Anar Manafov
                                  2007-06-01
        last changed by:   $LastChangedBy: manafov $ $LastChangedDate: 2007-06-01 18:47:22 +0200 (Fri, 01 Jun 2007) $

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef JOBSUBMITTER_H_
#define JOBSUBMITTER_H_

// Qt
#include <QThread>

// GAW
#include "gLiteAPIWrapper.h"

class CJobSubmitter: public QThread
{
        Q_OBJECT

        typedef glite_api_wrapper::CGLiteAPIWrapper GAW;

    public:
        CJobSubmitter( QObject *parent ):
                QThread(parent),
                m_JobsCount(0)
        {
            GAW::Instance().Init();
        }

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
                // Submit a Grid Job
                //TODO: take jdl from GUI
		GAW::Instance().GetJobManager().DelegationCredential();
		GAW::Instance().GetJobManager().JobSubmit( "gLitePROOF.jdl" );// TODO: check error
                emit changeProgress( i * 100 / m_JobsCount );
            }
            emit changeProgress( 100 );
        }

    private:
        size_t m_JobsCount;
};

#endif /*JOBSUBMITTER_H_*/
