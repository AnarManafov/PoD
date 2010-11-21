/************************************************************************/
/**
 * @file ServerDlg.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-08-24
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// Qt
#include <QWidget>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
// STD
#include <sstream>
// API
#include <signal.h>
// MiscCommon
#include "INet.h"
#include "Process.h"
// PAConsole
#include "ServerDlg.h"
#include "ServerInfo.h"
#include "version.h"
//=============================================================================
const size_t g_WaitTimeout = 10; // in sec.
const int g_defaultUpdTime = 10000; // in ms.
//=============================================================================
using namespace std;
using namespace MiscCommon::INet;
using namespace MiscCommon;
//=============================================================================
CServerDlg::CServerDlg( QWidget *_parent ):
    QWidget( _parent )
{
    m_ui.setupUi( this );

    // Enabling timer which checks Server's socket availability
    m_updTimer = new QTimer( this );
    connect( m_updTimer, SIGNAL( timeout() ), this, SLOT( update_check_srv_socket() ) );
    m_updTimer->setInterval( g_defaultUpdTime );

    update_check_srv_socket( true );
}
//=============================================================================
CServerDlg::~CServerDlg()
{
}
//=============================================================================
void CServerDlg::CommandServer( EServerCommands _command )
{
    string cmd( "$POD_LOCATION/bin/pod-server" );
    smart_path( &cmd );
    StringVector_t params;
    switch( _command )
    {
        case srvSTART:
            params.push_back( "start" );
            break;
        case srvSTOP:
            params.push_back( "stop" );
            break;
        default:
            return; //TODO: assert me!
    }
    try
    {
        string output;
        do_execv( cmd, params, 30, &output );
    }
    catch( const exception &_e )
    {
        QMessageBox::critical( this, tr( PROJECT_NAME ), tr( _e.what() ) );
    }

    update_check_srv_socket();
}
//=============================================================================
bool CServerDlg::IsRunning( bool _check_all )
{
    CServerInfo si;
    return si.IsRunning( _check_all );
}
//=============================================================================
void CServerDlg::on_btnStartServer_clicked()
{
    CommandServer( srvSTART );
    emit serverStart();
}
//=============================================================================
void CServerDlg::on_btnStopServer_clicked()
{
    CommandServer( srvSTOP );
}
//=============================================================================
void CServerDlg::update_check_srv_socket( bool _force )
{
    // Don't process if the page is hidden
    if( !_force && isHidden() )
        return;

    string cmd( "$POD_LOCATION/bin/pod-server" );
    smart_path( &cmd );
    StringVector_t params;
    params.push_back( "status" );
    string output;
    try
    {
        do_execv( cmd, params, g_WaitTimeout, &output );
    }
    catch( ... )
    {
    }
    m_ui.edtServerInfo->setText( QString( output.c_str() ) );
}
//=============================================================================
void CServerDlg::showEvent( QShowEvent* )
{
    update_check_srv_socket( true );
    m_updTimer->start();
}
//=============================================================================
void CServerDlg::hideEvent( QHideEvent* )
{
    m_updTimer->stop();
}
