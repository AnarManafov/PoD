/************************************************************************/
/**
 * @file JobSubmitter.h
 * @brief A thread-class which submits Grid jobs and reports a progress status
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-06-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef JOBSUBMITTER_H_
#define JOBSUBMITTER_H_

// Qt
#include <QtGui>

// MiscCommon
#include "gLiteHelper.h"

// GAW
#include "glite-api-wrapper/gLiteAPIWrapper.h"

class CJobSubmitter: public QThread
{
        Q_OBJECT

        typedef glite_api_wrapper::CGLiteAPIWrapper GAW;

    public:
        CJobSubmitter( QObject *parent ):
                QThread(parent)
        {
            GAW::Instance().Init();
        }
        ~CJobSubmitter()
        {
            if ( isRunning() )
                terminate();
        }

    public:
        const std::string &getLastJobID()
        {
            return m_LastJobID;
        }
        void setJDLFileName( const std::string &_JDLfilename )
        {
          m_JDLfilename = _JDLfilename;
        }

    signals:
        void changeProgress( int _Val);
        void changeNumberOfJobs( int _Val );
        void sendThreadMsg( const QString &_Msg );

    protected:
        void run()
        {
            emit changeProgress( 0 );
            m_mutex.lock();
            m_LastJobID.clear();
            m_mutex.unlock();

            // Submit a Grid Job
            //TODO: take jdl from GUI or ???
            try
            {
                GAW::Instance().GetJobManager().DelegationCredential();
                emit changeProgress( 30 );

                std::string sLastJobID;
                GAW::Instance().GetJobManager().JobSubmit( m_JDLfilename.c_str(), &sLastJobID );

                // Retrieving a number of children of the parametric job
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(sLastJobID).GetChildren( &jobs );
                emit changeNumberOfJobs( jobs.size() );
                m_mutex.lock();
                m_LastJobID = sLastJobID;
                m_mutex.unlock();
            }
            catch ( const std::exception &_e )
            {
                emit sendThreadMsg( tr(_e.what()) );
                emit changeProgress( 0 );
                return;
            }

            emit changeProgress( 100 );
        }

    private:
        std::string m_LastJobID;
        QMutex m_mutex;
        std::string m_JDLfilename;
};

#endif /*JOBSUBMITTER_H_*/
