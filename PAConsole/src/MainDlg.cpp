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
// STD
#include <fstream>
#include <exception>
// BOOST
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
// MiscCommon
#include "def.h"
// PAConsole
#include "MainDlg.h"
#include "ServerDlg.h"
#include "GridDlg.h"
#include "WorkersDlg.h"

using namespace std;
using namespace MiscCommon;

LPCTSTR g_szCfgFileName = "$GLITE_PROOF_LOCATION/etc/PAConsole.xml.cfg";

template<class T>
void _loadcfg( T &_s, string _FileName, QDialog *_Parent = NULL ) throw()
{
    smart_path(&_FileName);
    try
    {
        if (_FileName.empty() || !is_file_exists(_FileName))
            throw exception();

        ifstream f(_FileName.c_str());
        //assert(f.good());
        boost::archive::xml_iarchive ia(f);
        ia >> BOOST_SERIALIZATION_NVP(_s);
    }
    catch (...)
    {
        QMessageBox::warning(_Parent, "PROOFAgent Console",
                             "Can't load configuration from \"$GLITE_PROOF_LOCATION/etc/PAConsole.xml.cfg\"" );
    }
}

template<class T>
void _savecfg( const T &_s, string _FileName, QDialog *_Parent = NULL  ) throw()
{
    smart_path(&_FileName);
    try
    {
        if (_FileName.empty() || !is_file_exists(_FileName))
            throw exception();

        // make an archive
        ofstream f(_FileName.c_str());
        //assert(f.good());

        boost::archive::xml_oarchive oa(f);
        oa << BOOST_SERIALIZATION_NVP(_s);
    }
    catch (...)
    {
        QMessageBox::warning(_Parent, "PROOFAgent Console",
                             "Can't save configuration to \"$GLITE_PROOF_LOCATION/etc/PAConsole.xml.cfg\"" );
    }
}

CMainDlg::CMainDlg(QDialog *_Parent): QDialog( _Parent )
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

    // Loading class from the config file
    _loadcfg(*this, g_szCfgFileName, this);

    m_ui.pagesWidget->insertWidget( 0, &m_server);
    m_ui.pagesWidget->insertWidget( 1, &m_grid );
    m_ui.pagesWidget->insertWidget( 2, &m_workers );

    createIcons();
    m_ui.contentsWidget->setCurrentRow( 0 );

    connect( m_grid.getJobSubmitter(), SIGNAL(changeNumberOfJobs(int)), &m_workers, SLOT(setNumberOfJobs(int)) );
}

CMainDlg::~CMainDlg()
{
    // Saving class to the config file
    _savecfg(*this, g_szCfgFileName, this);
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
