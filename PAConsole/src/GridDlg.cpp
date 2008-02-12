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
#include <QTreeWidget>
// PAConsole
#include "GridDlg.h"
// GAW
#include "glite-api-wrapper/WMPEndpoint.h"
// MiscCommon
#include "JDLHelper.h"

const size_t g_TimeoutRefreshrate = 5000;
// default JDL file
const char * const g_szDefaultJDL = "$GLITE_PROOF_LOCATION/etc/gLitePROOF.jdl";

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::gLite;
using namespace glite_api_wrapper;

CGridDlg::CGridDlg( QWidget *parent ): QWidget( parent )
{
    m_ui.setupUi( this );

    m_JobSubmitter = JobSubmitterPtr_t( new CJobSubmitter( this ) );

    m_Timer = new QTimer(this);
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(updateJobsTree()) );
    m_Timer->start( g_TimeoutRefreshrate );

    connect( m_JobSubmitter.get(), SIGNAL(changeProgress(int)), this, SLOT(setProgress(int)) );
    connect( m_JobSubmitter.get(), SIGNAL(sendThreadMsg(const QString&)), this, SLOT(recieveThreadMsg(const QString&)) );

    createActions();

    clipboard = QApplication::clipboard();

    UpdateEndpoints();

    // Set completion for the edit box of JDL file name
    QCompleter *completer = new QCompleter( this );
    completer->setModel( new QDirModel(completer) );
    m_ui.edtJDLFileName->setCompleter(completer);
    string jdl( g_szDefaultJDL );
    smart_path( &jdl );
    m_ui.edtJDLFileName->setText( jdl.c_str() );

    // Setting up PARAMETERS
    int num_jobs(0);
    try
    {
        get_ad_attr( &num_jobs, m_ui.edtJDLFileName->text().toAscii().data(), JDL_PARAMETERS );
    }
    catch (...)
    {
    }
    m_ui.spinNumWorkers->setValue( num_jobs );
}

CGridDlg::~CGridDlg()
{
    if ( m_Timer )
    {
        m_Timer->stop();
        delete m_Timer;
    }
}

void CGridDlg::recieveThreadMsg( const QString &_Msg)
{
    QMessageBox::critical( this, tr("PROOFAgent Console"), _Msg );
    m_ui.btnSubmitClient->setEnabled( true );
}

void CGridDlg::on_btnSubmitClient_clicked()
{
    if ( !m_JobSubmitter->isRunning() )
    {
        if ( !QFileInfo( m_ui.edtJDLFileName->text() ).exists() )
        {
            QMessageBox::critical( this, tr("PROOFAgent Console"), tr("File\n\"%1\"\ndoesn't exist!").arg(m_ui.edtJDLFileName->text()) );
            return;
        }
        // Setting up PARAMETERS
        set_ad_attr( m_ui.spinNumWorkers->value(), m_ui.edtJDLFileName->text().toAscii().data(), JDL_PARAMETERS );

        m_JobSubmitter->setJDLFileName( m_ui.edtJDLFileName->text().toAscii().data() );
        m_JobSubmitter->setEndpoint( m_ui.cmbEndpoint->currentText().toAscii().data() );
        // submit gLite jobs
        m_JobSubmitter->start();
        m_ui.btnSubmitClient->setEnabled( false );
    }
    else
    {
        // Job submitter's thread
        m_JobSubmitter->terminate();
        setProgress( 0 );
        m_ui.btnSubmitClient->setEnabled( true );
    }
}

void CGridDlg::updateJobsTree()
{
    m_TreeItems.update( m_JobSubmitter->getLastJobID(), m_ui.treeJobs );
}

void CGridDlg::on_btnBrowseJDL_clicked()
{
    const QString dir = QFileInfo( m_ui.edtJDLFileName->text() ).absolutePath();
    const QString filename = QFileDialog::getOpenFileName(this,
                             tr("Select a jdl file"),
                             dir,
                             tr("JDL Files (*.jdl)"));
    if ( QFileInfo(filename).exists() )
        m_ui.edtJDLFileName->setText(filename);
}

void CGridDlg::createActions()
{
    // COPY Job ID
    copyJobIDAct = new QAction(tr("&Copy JobID"), this);
    copyJobIDAct->setShortcut(tr("Ctrl+C"));
    copyJobIDAct->setStatusTip(tr("Copy selected Jod ID to the clipboard"));
    connect( copyJobIDAct, SIGNAL(triggered()), this, SLOT(copyJobID()) );
    // CANCEL Job
    cancelJobAct = new QAction(tr("Canc&el Job"), this);
    cancelJobAct->setShortcut(tr("Ctrl+E"));
    cancelJobAct->setStatusTip(tr("Cancel selected Jod"));
    connect( cancelJobAct, SIGNAL(triggered()), this, SLOT(cancelJob()) );
}

void CGridDlg::contextMenuEvent( QContextMenuEvent *event )
{
    // Checking that *treeJobs* has been selected
    QPoint pos = event->globalPos();
    if ( !m_ui.treeJobs->childrenRect().contains( m_ui.treeJobs->mapFromGlobal(pos) ) )
        return;

    // We need to disable menu items when no jobID is selected
    const QTreeWidgetItem * item( m_ui.treeJobs->currentItem() );

    QMenu menu(this);
    menu.addAction(copyJobIDAct);
    copyJobIDAct->setEnabled( item );

    menu.addSeparator();
    menu.addAction(cancelJobAct);
    cancelJobAct->setEnabled( item );

    menu.exec(event->globalPos());
}

void CGridDlg::copyJobID() const
{
    // Copy selected JobID to clipboard
    const QTreeWidgetItem * item( m_ui.treeJobs->currentItem() );
    if ( !item )
        return;
    clipboard->setText( item->text(0), QClipboard::Clipboard);
    clipboard->setText( item->text(0), QClipboard::Selection);
}

void CGridDlg::cancelJob()
{
    const QTreeWidgetItem * item( m_ui.treeJobs->currentItem() );
    if ( !item )
        return;

    const string jobid( item->text(0).toAscii().data() );
    const string msg( "Do you really want to cancel the job:\n" + jobid );
    const QMessageBox::StandardButton reply( QMessageBox::question( this, tr("PROOFAgent Console"), tr(msg.c_str()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel ) );
    if ( QMessageBox::Yes != reply )
        return;

    try
    {
        CGLiteAPIWrapper::Instance().GetJobManager().JobCancel( jobid );
    }
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr("PROOFAgent Console"), tr(_e.what()) );
    }
}

void CGridDlg::setProgress( int _Val )
{
    if ( 100 == _Val )
        m_ui.btnSubmitClient->setEnabled( true );
    m_ui.progressSubmittedJobs->setValue( _Val );
}

// Retrieving a list of possiable WMProxy endpoints
void CGridDlg::UpdateEndpoints()
{
    m_ui.cmbEndpoint->clear();

    CWMPEndpoint endpoint;
    StringVector_t endpoints;
    try
    {
        endpoint.Get( &endpoints, m_ui.edtJDLFileName->text().toAscii().data() );
    }
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr("PROOFAgent Console"), tr(_e.what()) );
    }

    // Let's fill the Combobox
    StringVector_t::const_iterator iter( endpoints.begin() );
    StringVector_t::const_iterator iter_end( endpoints.end() );
    for ( ; iter != iter_end; ++iter )
    {
        m_ui.cmbEndpoint->addItem( tr(iter->c_str()) );
    }
}

void CGridDlg::on_edtJDLFileName_textChanged( const QString &/*_text*/ )
{
    UpdateEndpoints();
}
