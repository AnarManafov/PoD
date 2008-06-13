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
#include "SysHelper.h"

const size_t g_TimeoutRefreshrate = 3000; // in milliseconds
// default JDL file
const char * const g_szDefaultJDL = "$GLITE_PROOF_LOCATION/etc/gLitePROOF.jdl";

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::gLite;
using namespace glite_api_wrapper;

typedef glite_api_wrapper::CJobManager::delivered_output_t gaw_path_type;

string gaw_path_type_to_string( const gaw_path_type::value_type &_joboutput_path )
{
    string str("output of ");
    str += _joboutput_path.first;
    str += " has been saved to ";
    str += _joboutput_path.second;
    return str;
}

CGridDlg::CGridDlg( QWidget *parent ):
        QWidget( parent ),
        m_JobSubmitter(this)
{
    m_ui.setupUi( this );

    m_Timer = new QTimer(this);
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(updateJobsTree()) );
    m_Timer->start( g_TimeoutRefreshrate );

    connect( &m_JobSubmitter, SIGNAL(changeProgress(int)), this, SLOT(setProgress(int)) );
    connect( &m_JobSubmitter, SIGNAL(sendThreadMsg(const QString&)), this, SLOT(recieveThreadMsg(const QString&)) );

    createActions();

    clipboard = QApplication::clipboard();

    // Set completion for the edit box of JDL file name
    QCompleter *completer = new QCompleter( this );
    completer->setModel( new QDirModel(completer) );
    m_ui.edtJDLFileName->setCompleter(completer);
}

CGridDlg::~CGridDlg()
{
    if ( m_Timer )
    {
        m_Timer->stop();
        delete m_Timer;
    }
}

void CGridDlg::setAllDefault()
{
    m_JDLFileName = g_szDefaultJDL;
    m_JobSubmitter.setAllDefault();
    UpdateAfterLoad();
}

void CGridDlg::UpdateAfterLoad()
{
    smart_path( &m_JDLFileName );
    m_ui.edtJDLFileName->setText( m_JDLFileName.c_str() );

    // Setting up PARAMETERS
    int num_jobs(0);
    try
    {
        get_ad_attr( &num_jobs, m_JDLFileName, JDL_PARAMETERS );
    }
    catch (...)
    {
    }
    m_ui.spinNumWorkers->setValue( num_jobs );

    updateJobsTree();
}

void CGridDlg::recieveThreadMsg( const QString &_Msg)
{
    QMessageBox::critical( this, tr("PROOFAgent Console"), _Msg );
    m_ui.btnSubmitClient->setEnabled( true );
}

void CGridDlg::on_btnSubmitClient_clicked()
{
    if ( !m_JobSubmitter.isRunning() )
    {
        if ( !QFileInfo( m_ui.edtJDLFileName->text() ).exists() )
        {
            QMessageBox::critical( this, tr("PROOFAgent Console"), tr("File\n\"%1\"\ndoesn't exist!").arg(m_ui.edtJDLFileName->text()) );
            return;
        }
        // Setting up PARAMETERS
        set_ad_attr( m_ui.spinNumWorkers->value(), m_JDLFileName, JDL_PARAMETERS );

        m_JobSubmitter.setJDLFileName( m_JDLFileName );
        m_JobSubmitter.setEndpoint( m_ui.cmbEndpoint->currentText().toAscii().data() );
        // submit gLite jobs
        m_JobSubmitter.start();
        m_ui.btnSubmitClient->setEnabled( false );
    }
    else
    {
        // Job submitter's thread
        m_JobSubmitter.terminate();
        setProgress( 0 );
        m_ui.btnSubmitClient->setEnabled( true );
    }
}

void CGridDlg::updateJobsTree()
{
    m_TreeItems.update( m_JobSubmitter.getLastJobID(), m_ui.treeJobs );
}

void CGridDlg::on_btnBrowseJDL_clicked()
{
    const QString dir = QFileInfo( m_ui.edtJDLFileName->text() ).absolutePath();
    const QString filename = QFileDialog::getOpenFileName(this,
                             tr("Select a jdl file"),
                             dir,
                             tr("JDL Files (*.jdl)"));
    if ( QFileInfo(filename).exists() )
    {
        m_JDLFileName = filename.toAscii().data();
        m_ui.edtJDLFileName->setText(filename);
    }
}

void CGridDlg::createActions()
{
    // COPY Job ID
    copyJobIDAct = new QAction(tr("&Copy JobID"), this);
    copyJobIDAct->setShortcut(tr("Ctrl+C"));
    copyJobIDAct->setStatusTip(tr("Copy selected jod id to the clipboard"));
    connect( copyJobIDAct, SIGNAL(triggered()), this, SLOT(copyJobID()) );
    // CANCEL Job
    cancelJobAct = new QAction(tr("Canc&el Job"), this);
    cancelJobAct->setShortcut(tr("Ctrl+E"));
    cancelJobAct->setStatusTip(tr("Cancel the selected jod"));
    connect( cancelJobAct, SIGNAL(triggered()), this, SLOT(cancelJob()) );
    // GET OUTPUT of the Job
    getJobOutputAct = new QAction(tr("Get &output"), this);
    getJobOutputAct->setShortcut(tr("Ctrl+O"));
    getJobOutputAct->setStatusTip(tr("Get output sandbox of the selected jod"));
    connect( getJobOutputAct, SIGNAL(triggered()), this, SLOT(getJobOutput()) );
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

    menu.addAction(getJobOutputAct);
    getJobOutputAct->setEnabled( item );

    menu.addSeparator();
    menu.addAction(cancelJobAct);
    cancelJobAct->setEnabled( item );

    menu.exec(event->globalPos());
}

void CGridDlg::copyJobID() const
{
    // Copy selected JobID to clipboard
    const QTreeWidgetItem *item( m_ui.treeJobs->currentItem() );
    if ( !item )
        return;
    clipboard->setText( item->text(0), QClipboard::Clipboard);
    clipboard->setText( item->text(0), QClipboard::Selection);
}

void CGridDlg::cancelJob()
{
    const QTreeWidgetItem *item( m_ui.treeJobs->currentItem() );
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
        return;
    }
}

void CGridDlg::getJobOutput()
{
    const QTreeWidgetItem *item( m_ui.treeJobs->currentItem() );
    if ( !item )
        return;

    bool ok;
    const QString path = QInputDialog::getText(this, tr("PROOFAgent Console"),
                         tr("Enter the path, where output files should be delivered to:"),
                         QLineEdit::Normal,
                         QDir::home().absolutePath(), &ok);
    if (!ok || path.isEmpty())
        return;

    const string jobid( item->text(0).toAscii().data() );

    gaw_path_type joboutput_path;
    try
    {
        CGLiteAPIWrapper::Instance().GetJobManager().JobOutput(
            jobid,
            path.toAscii().data(),
            &joboutput_path,
            true);
    }
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr("PROOFAgent Console"), tr(_e.what()) );
        return;
    }

    ostringstream ss;
    transform( joboutput_path.begin(), joboutput_path.end(),
               ostream_iterator<string>(ss, "\n____\n"),
               gaw_path_type_to_string);
    QMessageBox::information( this, tr("PROOFAgent Console"), tr(ss.str().c_str()) );
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
        endpoint.Get( &endpoints, m_JDLFileName );
    }
    catch ( const exception &_e )
    {
        QMessageBox::critical( this, tr("PROOFAgent Console"), tr(_e.what()) );
        return;
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
