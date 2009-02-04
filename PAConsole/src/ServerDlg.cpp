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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
// Qt
#include <QWidget>
#include <QtXml/QDomDocument>
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

const size_t g_TimeoutCheckSrvSocket = 2000;  // in milliseconds
const char * const g_szPROOF_CFG = "$GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml";
// default pid/log directory
const char * const g_szPID_Dir = "$GLITE_PROOF_LOCATION/";

using namespace std;
using namespace MiscCommon::INet;
using namespace MiscCommon;

CServerDlg::CServerDlg( QWidget *_parent ):
        QWidget( _parent ),
        m_PIDDir( g_szPID_Dir )
{
    m_ui.setupUi( this );

    // PROOFAgent server's Port number
    getSrvPort( &m_SrvPort );

    // Enabling timer which checks Server's socket availability
    m_TimerSrvSocket = new QTimer( this );
    connect( m_TimerSrvSocket, SIGNAL( timeout() ), this, SLOT( update_check_srv_socket() ) );
    m_TimerSrvSocket->start( g_TimeoutCheckSrvSocket );

    // pid/log directory
    smart_path( &m_PIDDir );
    m_ui.edtPIDDir->setText( m_PIDDir.c_str() );

    // Show status on start-up
    on_btnStatusServer_clicked();
}

CServerDlg::~CServerDlg()
{
    if ( m_TimerSrvSocket )
    {
        m_TimerSrvSocket->stop();
        delete m_TimerSrvSocket;
    }
}

void CServerDlg::on_btnStatusServer_clicked()
{
    const QColor c = ( IsRunning( true ) ) ? QColor( 0, 0, 255 ) : QColor( 255, 0, 0 );
    m_ui.edtServerInfo->setTextColor( c );

    CServerInfo si;
    stringstream ss;
    ss
    << si.GetXROOTDInfo() << "\n"
    << "-----------------------\n"
    << si.GetPAInfo() << "\n";

    m_ui.edtServerInfo->setText( QString( ss.str().c_str() ) );
}

void CServerDlg::CommandServer( EServerCommands _command )
{
    string cmd( "$GLITE_PROOF_LOCATION/bin/Server_PoD.sh" );
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
        do_execv( cmd, params, 30, false );
    }
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr( "PROOFAgent Console" ), tr( _e.what() ) );
    }
}

bool CServerDlg::IsRunning( bool _check_all )
{
    CServerInfo si;
    return si.IsRunning( _check_all );
}

void CServerDlg::on_btnStartServer_clicked()
{
    if ( IsRunning( true ) )
        return;

    CommandServer( srvSTART );

    if ( !IsRunning( true ) )
    {
    	QString msg = tr( "<p>An error occurred while starting the Server!" );
    	char *chRootsys = getenv("ROOTSYS");
    	CServerInfo si;
    	// TODO: we need check processes and its user name, not only existance of the process
    	if( /*!si.IsXROOTDRunning() && */!chRootsys )
    		msg += "\n PAConsole has detected that $ROOTSYS is not set. If you use xrootd from a ROOT instalaltion,\n"
    			"then you need to set ROOTSYS before starting PACosnole.";
        QMessageBox::critical( this, tr( "PROOFAgent Console" ), msg );
    }

    on_btnStatusServer_clicked();
}

void CServerDlg::on_btnStopServer_clicked()
{
    if ( !IsRunning( false ) )
        return;

    CommandServer( srvSTOP );
    if ( IsRunning( true ) )
        QMessageBox::critical( this, tr( "PROOFAgent Console" ), tr( "<p>An error occurred while stopping the Server!" ) );

    on_btnStatusServer_clicked();
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

void CServerDlg::update_check_srv_socket()
{
    m_ui.btnStartServer->setEnabled( get_free_port( m_SrvPort ) );
}

void CServerDlg::getSrvPort( int *_Port )
{
    if ( !_Port )
        return ;

    string cfg( g_szPROOF_CFG );
    smart_path( &cfg );

    QFile file( cfg.c_str() );
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::warning( this, tr( "PROOFAgent Console" ),
                              tr( "Cannot read file %1:\n%2." )
                              .arg( cfg.c_str() )
                              .arg( file.errorString() ) );
        return ;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !domDocument.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
        QMessageBox::information( window(), tr( "PROOFAgent Console" ),
                                  tr( "Parse error at line %1, column %2:\n%3" )
                                  .arg( errorLine )
                                  .arg( errorColumn )
                                  .arg( errorStr ) );
        return ;
    }
    QDomNodeList instance = domDocument.elementsByTagName( "instance" );
    if ( instance.isEmpty() )
        return; // TODO: msg me!

    QDomNode server;
    for ( int i = 0; i < instance.count(); ++i )
    {
        const QDomNode name(
            instance.item( i ).attributes().namedItem( "name" ) );
        if ( name.isNull() )
            continue;
        // TODO: We look exactly for "server" instance!
        // Change it. Move instance name to the PAConsole settings.
        if ( name.nodeValue() == "server" )
        {
            server = instance.item( i );
            break;
        }
    }

    QDomNode agent_server = server.namedItem( "agent_server" );
    if ( agent_server.isNull() )
        return ; // TODO: Msg me!

    QDomNode port = agent_server.namedItem( "listen_port" ).firstChild();
    if ( port.isNull() )
        return ; // TODO: Msg me!

    *_Port = port.nodeValue().toInt();
}
