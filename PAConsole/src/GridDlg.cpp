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

using namespace std;
using namespace MiscCommon;
using namespace MiscCommon::gLite;
using namespace glite_api_wrapper;

CGridDlg::CGridDlg( QWidget *parent ): QWidget( parent )
{
    m_ui.setupUi( this );

    m_JobSubmitter = JobSubmitterPtr_t( new CJobSubmitter( this ) );

    updateJobsTree(); // TODO: set this method to timer events or refresh btn
}

CGridDlg::~CGridDlg()
{
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
    string sLastJobID( "https://grid25.gsi.de:9000/ZorYI4-d-rm2MghGEUsL1Q" );//m_JobSubmitter->getLastJobID() );

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

            QTreeWidgetItem *item = new QTreeWidgetItem( parentJob );
            item->setText( 0, iter->c_str() );
            item->setText( 1, status.c_str() );
        }
        m_ui.treeJobs->setColumnWidth( 0, 260 );
        m_ui.treeJobs->expandAll();
    }
    catch ( const exception &_e)
    {
        // TODO: Msg me!
    }
}
