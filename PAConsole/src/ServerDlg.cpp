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
// Qt
#include <QWidget>
#include <QtXml/QDomDocument>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

// STD
#include <sstream>

// API
#include <signal.h>

// PAConsole
#include "ServerDlg.h"
#include "ServerInfo.h"
#include "INet.h"

const size_t g_TimeoutCheckSrvSocket = 2000;

using namespace std;
using namespace MiscCommon::INet;

CServerDlg::CServerDlg( QWidget *parent ): QWidget( parent )
{
    m_ui.setupUi( this );

    // PROOFAgent server's Port number
    getSrvPort( &m_SrvPort );

    // Enabling timer which checks Server's socket availability
    m_TimerSrvSocket = new QTimer(this);
    connect( m_TimerSrvSocket, SIGNAL(timeout()), this, SLOT(update_check_srv_socket()) );
    m_TimerSrvSocket->start(g_TimeoutCheckSrvSocket);

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
    const QColor c = ( IsRunning() ) ? QColor(0, 0, 255) : QColor(255, 0, 0);
    m_ui.edtServerInfo->setTextColor( c );

    CServerInfo si;
    stringstream ss;
    ss
    << si.GetXROOTDInfo() << "\n"
    << si.GetPAInfo() << "\n";

    m_ui.edtServerInfo->setText( QString(ss.str().c_str()) );
}

void CServerDlg::Stop()
{
    const string cmd = string("./Server_gLitePROOF.sh ") + m_ui.edtPIDDir->text().toAscii().data() + string(" stop");
    int res = system( cmd.c_str() );
    // HACK: warning: ignoring return value of `int system(const char*)`, declared with attribute warn_unused_result
    if (WIFSIGNALED(res) &&
        (WTERMSIG(res) == SIGINT || WTERMSIG(res) == SIGQUIT))
        return; //break
}

void CServerDlg::Start()
{
    const string cmd = string("./Server_gLitePROOF.sh ") + m_ui.edtPIDDir->text().toAscii().data() + string(" start");
    int res = system( cmd.c_str() );
    // HACK: warning: ignoring return value of `int system(const char*)`, declared with attribute warn_unused_result
    if (WIFSIGNALED(res) &&
        (WTERMSIG(res) == SIGINT || WTERMSIG(res) == SIGQUIT))
        return; //break;
}

bool CServerDlg::IsRunning()
{
    CServerInfo si;
    const pid_t pidXrootD = si.IsXROOTDRunning();
    const pid_t pidPA = si.IsPROOFAgentRunning();
    return  (pidXrootD && pidPA);
}

void CServerDlg::on_btnStartServer_clicked()
{
    if ( IsRunning() )
        Stop();
    else
        Start();

    if ( !IsRunning() )
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while starting the Server!") );

    on_btnStatusServer_clicked();
}

void CServerDlg::on_btnStopServer_clicked()
{
    if ( !IsRunning() )
        return;

    Stop();
    if ( IsRunning() )
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while stopping the Server!") );

    on_btnStatusServer_clicked();
}

void CServerDlg::on_btnBrowsePIDDir_clicked()
{
    const QString directory = QFileDialog::getExistingDirectory(this,
                              tr("Select pid directory of PROOFAgent"),
                              m_ui.edtPIDDir->text(),
                              QFileDialog::DontResolveSymlinks
                              | QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.edtPIDDir->setText(directory);
}

void CServerDlg::update_check_srv_socket()
{
    m_ui.btnStartServer->setEnabled( get_free_port(m_SrvPort) );
}

void CServerDlg::getSrvPort( int *_Port )
{
    if ( !_Port )
        return ;

    QFile file("./proofagent.cfg.xml");
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("PROOFAgent Console"),
                             tr("Cannot read file %1:\n%2.")
                             .arg("./proofagent.cfg.xml")
                             .arg(file.errorString()));
        return ;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !domDocument.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
        QMessageBox::information(window(), tr("PROOFAgent Console"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return ;
    }
    QDomNodeList instance = domDocument.elementsByTagName("instance");
    if ( instance.isEmpty() )
        return; // TODO: msg me!

    QDomNode server;
    for ( int i = 0; i < instance.count(); ++i )
    {
        const QDomNode name(
            instance.item(i).attributes().namedItem("name") );
        if ( name.isNull() )
            continue;
        // TODO: We look exactly for "server" instance!
        // Change it. Move instance name to the PAConsole settings.
        if ( name.nodeValue() == "server" )
        {
            server = instance.item(i);
            break;
        }
    }

    QDomNode agent_server = server.namedItem("agent_server");
    if ( agent_server.isNull())
        return ; // TODO: Msg me!

    QDomNode port = agent_server.namedItem("listen_port").firstChild();
    if ( port.isNull())
        return ; // TODO: Msg me!

    *_Port = port.nodeValue().toInt();
}
