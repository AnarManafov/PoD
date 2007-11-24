/************************************************************************/
/**
 * @file MainDlg.cpp
 * @brief Main dialog implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-05-23
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
// Qt
#include <QtGui>
#include <QtUiTools/QUiLoader>

// PAConsole
#include "MainDlg.h"
#include "ServerDlg.h"
#include "GridDlg.h"
#include "WorkersDlg.h"

CMainDlg::CMainDlg(QDialog *_Parent):
  QDialog( _Parent )
{
    m_ui.setupUi( this );

    Qt::WindowFlags flags( 0 );
    // Adding "Maximize" and "Minimize" buttons
    // Removing "What's this" button
    flags |= Qt::WindowTitleHint;
    flags |= Qt::WindowSystemMenuHint;
    flags |= Qt::WindowMaximizeButtonHint;
    flags |= Qt::WindowMinimizeButtonHint;
    setWindowFlags( flags );
    
    CGridDlg *grid = new CGridDlg();
    CWorkersDlg *workers = new CWorkersDlg(); 
    m_ui.pagesWidget->insertWidget( 0, new CServerDlg );
    m_ui.pagesWidget->insertWidget( 1, grid );
    m_ui.pagesWidget->insertWidget( 2, workers );
    
    createIcons();
    m_ui.contentsWidget->setCurrentRow( 0 );
    
    connect( grid->getJobSubmitter(), SIGNAL(changeNumberOfJobs(int)), workers, SLOT(setNumberOfJobs(int)) );
}

void CMainDlg::createIcons()
{
    QListWidgetItem *configButton = new QListWidgetItem( m_ui.contentsWidget );
    configButton->setIcon(QIcon(":/images/server.png"));
    configButton->setText(tr("Server"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem( m_ui.contentsWidget );
    updateButton->setIcon(QIcon(":/images/grid.png"));
    updateButton->setText(tr("Grid"));
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *queryButton = new QListWidgetItem( m_ui.contentsWidget );
    queryButton->setIcon(QIcon(":/images/workers.png"));
    queryButton->setText(tr("Workers"));
    queryButton->setTextAlignment(Qt::AlignHCenter);
    queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect( m_ui.contentsWidget,
             SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
             this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}

void CMainDlg::changePage(QListWidgetItem *_Current, QListWidgetItem *_Previous)
{
    if ( !_Current )
        _Current = _Previous;

    m_ui.pagesWidget->setCurrentIndex( m_ui.contentsWidget->row(_Current) );
}
