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
    CServerInfo si;
    const pid_t pidXrootD = si.IsXROOTDRunning();
    const pid_t pidPA = si.IsPROOFAgentRunning();

    const QColor c = ( !pidXrootD || !pidPA ) ? QColor(255, 0, 0) : QColor(0, 0, 255);
    m_ui.edtServerInfo->setTextColor( c );

    stringstream ss;
    ss
    << si.GetXROOTDInfo() << "\n"
    << si.GetPAInfo() << "\n";

    m_ui.edtServerInfo->setText( QString(ss.str().c_str()) );
}

void CServerDlg::on_btnStartServer_clicked()
{
    CServerInfo si;
    pid_t pidXrootD = si.IsXROOTDRunning();
    pid_t pidPA = si.IsPROOFAgentRunning();
    if ( pidXrootD || pidPA )
    {
        QMessageBox::information(this, tr("PROOFAgent Console"), tr("<p>One or more components of the Server are running!<p> Please stop the Server first.") );
        return ;
    }

    const string cmd = string("./Server_gLitePROOF.sh ") + m_ui.edtPIDDir->text().toAscii().data() + string(" start");
    system( cmd.c_str() );
    pidXrootD = si.IsXROOTDRunning();
    pidPA = si.IsPROOFAgentRunning();
    if ( !pidXrootD || !pidPA )
    {
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while starting the Server!") );
        return ;
    }

    on_btnStatusServer_clicked();
}

void CServerDlg::on_btnStopServer_clicked()
{
    CServerInfo si;
    pid_t pidXrootD = si.IsXROOTDRunning();
    pid_t pidPA = si.IsPROOFAgentRunning();
    if ( !pidXrootD && !pidPA )
        return ;

    const string cmd = string("./Server_gLitePROOF.sh ") + m_ui.edtPIDDir->text().toAscii().data() + string(" stop");
    system( cmd.c_str() );
    pidXrootD = si.IsXROOTDRunning();
    pidPA = si.IsPROOFAgentRunning();
    if ( pidXrootD || pidPA )
    {
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while stopping the Server!") );
        return ;
    }

    /*    // Stoping Clients as well
        if ( m_ui.btnSubmitClient->isChecked() )
            m_ui.btnSubmitClient->click();
        // cleaning Clients list box
        m_ui.lstClientsList->clear();*/

    on_btnStatusServer_clicked();
}

void CServerDlg::on_btnBrowsePIDDir_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
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
    QDomNodeList server = domDocument.elementsByTagName("server");
    QDomNode node = server.at(0).namedItem("config");
    if ( node.isNull())
        return ; // TODO: Msg me!

    QDomNode nodeSrv = server.at(0).namedItem("agent_server");
    if ( node.isNull())
        return ; // TODO: Msg me!

    QDomNode port = nodeSrv.attributes().namedItem("listen_port");
    if ( port.isNull())
        return ; // TODO: Msg me!

    *_Port = port.nodeValue().toInt();
}
