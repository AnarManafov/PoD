/************************************************************************/
/**
 * @file MainDlg.h
 * @brief Main dialog declaration
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-05-23
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef CMAINDLG_H_
#define CMAINDLG_H_

// Qt
#include <QTimer>

// Qt autogen. file
#include "ui_maindlg.h"

// OUR
#include "MiscUtils.h"
#include "JobSubmitter.h"

typedef std::auto_ptr<CJobSubmitter> JobSubmitterPtr_t;

class CMainDlg: public QDialog
{
        Q_OBJECT

    public:
        CMainDlg( QDialog *_Parent = 0 );
        ~CMainDlg()
        {
            if ( m_Timer )
            {
                m_Timer->stop();
                delete m_Timer;
            }
            if ( m_TimerSrvSocket )
            {
                m_TimerSrvSocket->stop();
                delete m_TimerSrvSocket;
            }
        }

    private slots:
        // Server's slots
        void on_btnStatusServer_clicked();
        void on_btnStartServer_clicked();
        void on_btnStopServer_clicked();
        void on_btnBrowsePIDDir_clicked();
        // Client's slots
        void on_btnSubmitClient_clicked();
        // Timer
        void update();
        void update_check_srv_socket();
        // Progress
        void setProgress( int _Val )
        {
            if ( 100 == _Val )
                m_ui.btnSubmitClient->setEnabled( true );
            m_ui.progressSubmittedJobs->setValue( _Val );
        }
        // Monitor List of Workers
        void on_chkShowWorkers_stateChanged( int _Stat );

    private:
        void GetPROOFCfg( std::string *_FileName );
        void GetSrvPort( int *_Port );

    private:
        Ui::MainDlg m_ui;
        QTimer *m_Timer;
        QTimer *m_TimerSrvSocket;
        std::string m_CfgFileName;
        int m_SrvPort;
        JobSubmitterPtr_t m_JobSubmitter;
};


#endif /*CMAINDLG_H_*/
