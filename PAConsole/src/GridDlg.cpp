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
#include <QTreeWidget>

// GAW
#include "glite-api-wrapper/gLiteAPIWrapper.h"

// PAConsole
#include "GridDlg.h"

const size_t g_TimeoutRefreshrate = 5000;

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
}

void CGridDlg::on_btnSubmitClient_clicked()
{
    if ( !m_JobSubmitter->isRunning() )
    {
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
    m_ui.treeJobs->clear();
    string sLastJobID( m_JobSubmitter->getLastJobID() );

    if ( sLastJobID.empty() )
        return;

    QTreeWidgetItem *parentJob = new QTreeWidgetItem( m_ui.treeJobs );
    parentJob->setText( 0, sLastJobID.c_str() );

    try
    {
        StringVector_t jobs;
        CJobStatusObj(sLastJobID).GetChildren( &jobs );

        StringVector_t::const_iterator iter = jobs.begin();
        StringVector_t::const_iterator iter_end = jobs.end();
        for (; iter != iter_end; ++iter)
        {
            string status;
            CGLiteAPIWrapper::Instance().GetJobManager().JobStatus( *iter, &status );

            QTreeWidgetItem *item = new QTreeWidgetItem( parentJob ); // TODO: Investigate a memory management of QT objects. Do we leek here???
            item->setText( 0, iter->c_str() );
            item->setText( 1, status.c_str() );
        }
        m_ui.treeJobs->setColumnWidth( 0, 260 );
        m_ui.treeJobs->expandAll();
    }
    catch ( const exception &_e)
    {
        // TODO: Msg me! or..?
    }
}
