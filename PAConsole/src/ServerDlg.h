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
#ifndef SERVERDLG_H_
#define SERVERDLG_H_

// Qt
#include <QTimer>

// Qt autogen. file
#include "ui_wgServer.h"

class CServerDlg: public QWidget
{
        Q_OBJECT

    public:
        CServerDlg( QWidget *parent = 0 );
        virtual ~CServerDlg();

    private slots:
        void on_btnStatusServer_clicked();
        void on_btnStartServer_clicked();
        void on_btnStopServer_clicked();
        void on_btnBrowsePIDDir_clicked();

        void update_check_srv_socket();

    private:
        void getSrvPort( int *_Port );

    private:
        Ui::wgServer m_ui;

        int m_SrvPort;
        QTimer *m_TimerSrvSocket;
};

#endif /*SERVERDLG_H_*/
