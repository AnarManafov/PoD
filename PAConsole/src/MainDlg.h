/************************************************************************/
/**
 * @file MainDlg.h
 * @brief Main dialog declaration
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision: 931 $
        created by:          Anar Manafov
                                  2007-05-23
        last changed by:   $LastChangedBy: manafov $ $LastChangedDate: 2007-06-25 16:56:04 +0200 (Mon, 25 Jun 2007) $
 
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
#include "def.h"
#include "JobSubmitter.h"

typedef std::auto_ptr<CJobSubmitter> JobSubmitterPtr_t;

template <class _T>
struct SFindComment
{
    SFindComment( const _T &_CmntSign ): m_CmntSign(_CmntSign)
    {}
    bool operator() ( const _T &_Val ) const
    {
        return ( _Val.find(m_CmntSign) != _Val.npos );
    }
private:
    _T m_CmntSign;
};

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

        // Setting a number of connected workers        
        void setActiveWorkers( size_t _Val1, size_t _Val2 = 0 ) 
        {
            static size_t nTotal = 0;
            if( _Val2 )
                nTotal = _Val2; 
            MiscCommon::tstring strMsg( _T("Monitor connections (available %1 out of %2 worker(s)):") );
            MiscCommon::tstringstream ss;
            ss << _Val1;
            MiscCommon::replace<MiscCommon::tstring>( &strMsg, _T("%1"), ss.str() );
            ss.str("");
            ss << nTotal;
            MiscCommon::replace<MiscCommon::tstring>( &strMsg, _T("%2"), ss.str() );
            m_ui.chkShowWorkers->setText( strMsg.c_str() );
        }

    private:
        void getPROOFCfg( std::string *_FileName );
        void getSrvPort( int *_Port );

    private:
        Ui::MainDlg m_ui;
        QTimer *m_Timer;
        QTimer *m_TimerSrvSocket;
        std::string m_CfgFileName;
        int m_SrvPort;
        JobSubmitterPtr_t m_JobSubmitter;
};


#endif /*CMAINDLG_H_*/
