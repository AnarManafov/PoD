/************************************************************************/
/**
 * @file MainDlg.cpp
 * @brief Main dialog implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-05-23
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// Qt
#include <QtGui>
#include <QtUiTools/QUiLoader>
#include <QtXml/QDomDocument>

// STD
#include <sstream>
#include <fstream>

// Our
#include "MainDlg.h"
#include "ServerInfo.h"
#include "SysHelper.h"
#include "CustomIterator.h"
#include "INet.h"
#include "MiscUtils.h"

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::INet;

// TODO: Do we need to move it to the class?
const size_t g_TimeoutCheckSrvSocket = 2000;
const size_t g_TimeoutCheckPROOFCONF = 2500;

CMainDlg::CMainDlg(QDialog *_Parent):
        QDialog(_Parent)
{
    m_Timer = new QTimer(this);
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(update()) );

    // PROOFAgent server's Port number
    getSrvPort(&m_SrvPort);

    // Enabling timer which checks Server's socket availability
    m_TimerSrvSocket = new QTimer(this);
    connect( m_TimerSrvSocket, SIGNAL(timeout()), this, SLOT(update_check_srv_socket()) );
    m_TimerSrvSocket->start(g_TimeoutCheckSrvSocket);

    m_ui.setupUi( this );
    // Show status on start-up
    on_btnStatusServer_clicked();

    m_JobSubmitter = JobSubmitterPtr_t( new CJobSubmitter( this ) );

    connect( m_JobSubmitter.get(), SIGNAL(changeProgress(int)), this, SLOT(setProgress(int)) );

    on_chkShowWorkers_stateChanged( Qt::Checked );

    setActiveWorkers( 0 );
}

void CMainDlg::on_btnStatusServer_clicked()
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

void CMainDlg::on_btnStartServer_clicked()
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

void CMainDlg::on_btnStopServer_clicked()
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

    // Stoping Clients as well
    if ( m_ui.btnSubmitClient->isChecked() )
        m_ui.btnSubmitClient->click();
    // cleaning Clients list box
    m_ui.lstClientsList->clear();

    on_btnStatusServer_clicked();
}

void CMainDlg::on_btnBrowsePIDDir_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                        tr("Select pid directory of PROOFAgent"),
                        m_ui.edtPIDDir->text(),
                        QFileDialog::DontResolveSymlinks
                        | QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.edtPIDDir->setText(directory);
}

void CMainDlg::update()
{
    // Read proof.conf and update Listbox
    ifstream f( m_CfgFileName.c_str() );
    if ( !f.is_open() )
        return ;

    StringVector_t vec;

    copy(custom_istream_iterator<string>(f),
         custom_istream_iterator<string>(),
         back_inserter(vec));

    int cur_sel = m_ui.lstClientsList->currentRow();
    m_ui.lstClientsList->clear();

    // Reading only comment blocks of proof.conf
    const LPCTSTR chCmntSign("#");
    StringVector_t::iterator iter = find_if( vec.begin(), vec.end(),
                                    SFindComment<string>(chCmntSign) );
    StringVector_t::const_iterator iter_end = vec.end();
    while ( iter != iter_end )
    {
        trim_left( &*iter, string(chCmntSign) );
        m_ui.lstClientsList->addItem( iter->c_str() );
        iter = find_if( ++iter, vec.end(),
                        SFindComment<string>(chCmntSign) );
    }
    if ( m_ui.lstClientsList->count() >= cur_sel )
        m_ui.lstClientsList->setCurrentRow( cur_sel );

    setActiveWorkers( m_ui.lstClientsList->count() - 1 );
}

void CMainDlg::update_check_srv_socket()
{
    m_ui.btnStartServer->setEnabled( get_free_port(m_SrvPort) );
}

void CMainDlg::on_btnSubmitClient_clicked()
{
    if ( !m_JobSubmitter->isRunning() )
    {
        getPROOFCfg( &m_CfgFileName );
        smart_homedir_append( &m_CfgFileName );
        if ( m_CfgFileName.empty() )
        {
            QMessageBox::critical(this, tr("PROOFAgent Console"), tr("An Error occurred while retrieving proof.conf full name from proofagent.cfg.xml") );
            return ;
        }
        // submit gLite jobs
        m_JobSubmitter->set_jobs_count( m_ui.spinSubmitJobs->value() );
        m_JobSubmitter->start();
        m_ui.btnSubmitClient->setEnabled( false );

        setActiveWorkers( 0, m_ui.spinSubmitJobs->value() );
    }
    else
    {
        // Job submitter's thread
        m_JobSubmitter->terminate();
        setProgress( 0 );
        m_ui.btnSubmitClient->setEnabled( true );
    }
}

/// Simple method to read full name of the proof.conf from proofagent.cfg.xml (cfg file is hard-coded so far)
// TODO: Revise method and remove hard-coded reference to proofagent.cfg.xml
void CMainDlg::getPROOFCfg( string *_FileName )
{
    if ( !_FileName )
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

    QDomNode cfg = node.attributes().namedItem("proof_cfg_path");
    if ( cfg.isNull())
        return ; // TODO: Msg me!

    *_FileName = cfg.nodeValue().toAscii().data();
}

void CMainDlg::getSrvPort( int *_Port )
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

void CMainDlg::on_chkShowWorkers_stateChanged( int _Stat )
{
    const bool bEnable = (_Stat == Qt::Checked);
    if ( m_Timer->isActive() && bEnable )
        return ; // TODO: need assert here

    m_ui.lstClientsList->setEnabled( bEnable );

    if ( !bEnable )
        m_Timer->stop();
    else
        m_Timer->start(g_TimeoutCheckSrvSocket);
}
