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

// Qt autogen. file
#include "ui_wgServer.h"
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// MiscCommon
#include "SysHelper.h"

class QTimer;

class CServerDlg: public QWidget
{
    Q_OBJECT

    friend class boost::serialization::access;

public:
    CServerDlg( QWidget *_parent = NULL );
    virtual ~CServerDlg();

public:
    enum EServerCommands{ srvSTART, srvSTOP };

    void CommandServer( EServerCommands _command );

private slots:
    void on_btnStartServer_clicked();
    void on_btnStopServer_clicked();
    void on_btnBrowsePIDDir_clicked();
    void update_check_srv_socket( bool _force = false);

private:
    bool IsRunning( bool _check_all );

    template<class Archive>
    void save( Archive & _ar, const unsigned int /*_version*/ ) const
    {
        _ar & BOOST_SERIALIZATION_NVP( m_PIDDir );
    }
    template<class Archive>
    void load( Archive & _ar, const unsigned int /*_version*/ )
    {
        _ar & BOOST_SERIALIZATION_NVP( m_PIDDir );

        // pid/log directory
        MiscCommon::smart_path( &m_PIDDir );
        m_ui.edtPIDDir->setText( m_PIDDir.c_str() );
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    Ui::wgServer m_ui;
    int m_SrvPort;
    QTimer *m_Timer;
    std::string m_PIDDir;
};

BOOST_CLASS_VERSION( CServerDlg, 1 )

#endif /*SERVERDLG_H_*/
