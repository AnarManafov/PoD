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

// TODO: optimize the call of the status of PoD
// this is very expensive call, we therefore using 20 sec. timeout
const size_t g_UpdateInterval = 20000;  // in milliseconds
const size_t g_WaitTimeout = 15; // in sec.
// default pid/log directory
const char * const g_szPID_Dir = "$POD_LOCATION/";

using namespace std;
using namespace MiscCommon::INet;
using namespace MiscCommon;

CServerDlg::CServerDlg( QWidget *_parent ):
        QWidget( _parent ),
        m_PIDDir( g_szPID_Dir )
{
    m_ui.setupUi( this );

    // pid/log directory
    smart_path( &m_PIDDir );
    m_ui.edtPIDDir->setText( m_PIDDir.c_str() );

    // Enabling timer which checks Server's socket availability
    m_Timer = new QTimer( this );
    connect( m_Timer, SIGNAL( timeout() ), this, SLOT( update_check_srv_socket() ) );
    m_Timer->start( g_UpdateInterval );

    update_check_srv_socket( true );
}

CServerDlg::~CServerDlg()
{
}

void CServerDlg::CommandServer( EServerCommands _command )
{
    string cmd( "$POD_LOCATION/bin/Server_PoD.sh" );
    smart_path( &cmd );
    StringVector_t params;
    params.push_back( string( m_ui.edtPIDDir->text().toAscii().data() ) );
    switch ( _command )
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
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr( "PROOFAgent Console" ), tr( _e.what() ) );
    }

    update_check_srv_socket();
}

bool CServerDlg::IsRunning( bool _check_all )
{
    CServerInfo si;
    return si.IsRunning( _check_all );
}

void CServerDlg::on_btnStartServer_clicked()
{
    CommandServer( srvSTART );
}

void CServerDlg::on_btnStopServer_clicked()
{
    CommandServer( srvSTOP );
}

void CServerDlg::on_btnBrowsePIDDir_clicked()
{
    const QString directory = QFileDialog::getExistingDirectory( this,
                              tr( "Select pid directory of PROOFAgent" ),
                              m_ui.edtPIDDir->text(),
                              QFileDialog::DontResolveSymlinks
                              | QFileDialog::ShowDirsOnly );
    if ( !directory.isEmpty() )
    {
        m_ui.edtPIDDir->setText( directory );
        m_PIDDir = directory.toAscii().data();
    }
}

void CServerDlg::update_check_srv_socket( bool _force )
{
    // Don't process if the page is hidden
    if ( !_force && isHidden() )
        return;

    string cmd( "$POD_LOCATION/bin/Server_PoD.sh" );
    smart_path( &cmd );
    StringVector_t params;
    params.push_back( string( m_ui.edtPIDDir->text().toAscii().data() ) );
    params.push_back( "status" );
    string output;
    try
    {
        do_execv( cmd, params, g_WaitTimeout, &output );
    }
    catch (...)
    {
    }
    m_ui.edtServerInfo->setText( QString(output.c_str()) );
}
