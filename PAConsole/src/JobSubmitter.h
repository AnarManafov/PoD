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
#include <QThread>
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
        
    signals:
        void changeProgress( int _Val);
        void changeNumberOfJobs( int _Val, const std::string &_ParentJobID );

    protected:
        void run()
        {
            emit changeProgress( 0 );

            // Submit a Grid Job
            //TODO: take jdl from GUI
            try
            {
                GAW::Instance().GetJobManager().DelegationCredential();
                emit changeProgress( 30 );
                
                std::string m_LastJobID;
                GAW::Instance().GetJobManager().JobSubmit( "gLitePROOF.jdl", &m_LastJobID );// TODO: check error
                
                // Retrieving a number of children of the parametric job
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(m_LastJobID).GetChildren( &jobs );
                emit changeNumberOfJobs( jobs.size(), m_LastJobID );
            }
            catch ( const std::exception &_e )
            {
                return;
            }

            emit changeProgress( 100 );
        }
};

#endif /*JOBSUBMITTER_H_*/
