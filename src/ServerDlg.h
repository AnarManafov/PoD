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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef SERVERDLG_H_
#define SERVERDLG_H_
//=============================================================================
// Qt autogen. file
#include "ui_wgServer.h"
// MiscCommon
#include "SysHelper.h"
//=============================================================================
class QTimer;
//=============================================================================
class CServerDlg: public QWidget
{
        Q_OBJECT

    public:
        CServerDlg( QWidget *_parent = NULL );
        virtual ~CServerDlg();

        void show();

    public:
        enum EServerCommands { srvSTART, srvSTOP };

        void CommandServer( EServerCommands _command );

    public:
        QTimer *m_updTimer;

    signals:
        void serverStart();

    private slots:
        void on_btnStartServer_clicked();
        void on_btnStopServer_clicked();
        void update_check_srv_socket( bool _force = false );

    protected:
        void showEvent( QShowEvent* );
        void hideEvent( QHideEvent* );

    private:
        bool IsRunning( bool _check_all );

    private:
        Ui::wgServer m_ui;
        int m_SrvPort;
};

#endif /*SERVERDLG_H_*/
