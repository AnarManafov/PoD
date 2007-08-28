/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-08-24
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef GRIDDLG_H_
#define GRIDDLG_H_

// Qt autogen. file
#include "ui_wgGrid.h"

// MiscCommon
#include "def.h"

// PAConsole
#include "JobSubmitter.h"

typedef std::auto_ptr<CJobSubmitter> JobSubmitterPtr_t;

class CGridDlg: public QWidget
{
        Q_OBJECT

    public:
        CGridDlg( QWidget *parent = 0 );
        virtual ~CGridDlg();

    public:
        CJobSubmitter *getJobSubmitter()
        {
            return m_JobSubmitter.get();
        }

    public slots:
        void on_btnSubmitClient_clicked();
        void updateJobsTree();
        void recieveThreadMsg( const QString &_Msg);
        // Progress
        void setProgress( int _Val )
        {
            if ( 100 == _Val )
                m_ui.btnSubmitClient->setEnabled( true );
            m_ui.progressSubmittedJobs->setValue( _Val );
        }

    private:
        Ui::wgGrid m_ui;

        QTimer *m_Timer;
        JobSubmitterPtr_t m_JobSubmitter;
};

#endif /*GRIDDLG_H_*/
