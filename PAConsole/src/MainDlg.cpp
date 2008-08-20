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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
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

using namespace std;
using namespace MiscCommon;

LPCTSTR g_szCfgFileName = "$GLITE_PROOF_LOCATION/etc/PAConsole.xml.cfg";

template<class T>
void _loadcfg( T &_s, string _FileName )
{
    smart_path( &_FileName );
    if ( _FileName.empty() || !is_file_exists( _FileName ) )
        throw exception();

    ifstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_iarchive ia( f );
    ia >> BOOST_SERIALIZATION_NVP( _s );
}

template<class T>
void _savecfg( const T &_s, string _FileName )
{
    smart_path( &_FileName );
    if ( _FileName.empty() )
        throw exception();

    // make an archive
    ofstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_oarchive oa( f );
    oa << BOOST_SERIALIZATION_NVP( _s );
}

CMainDlg::CMainDlg( QDialog *_Parent ):
        QDialog( _Parent ),
        m_CurrentPage( 0 )
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

    try
    {
        // Loading class from the config file
        _loadcfg( *this, g_szCfgFileName );
    }
    catch ( ... )
    {
        m_grid.setAllDefault();
        QMessageBox::information( this, "PROOFAgent Console",
                                  "Can't load configuration from \
                                  \"$GLITE_PROOF_LOCATION / etc / PAConsole.xml.cfg\".\
                                  PAConsole will use its default settings." );
    }

    // Using preferences
    m_grid.restartUpdTimer( m_preferences.getJobStatusUpdInterval() );
    // Immediately update interval when a user changes settings
    connect( &m_preferences, SIGNAL( changedJobStatusUpdInterval( int ) ), &m_grid, SLOT( restartUpdTimer( int ) ) );
    m_workers.restartUpdTimer( m_preferences.getWorkersUpdInterval() );
    // Immediately update interval when a user changes settings
    connect( &m_preferences, SIGNAL( changedWorkersUpdInterval( int ) ), &m_workers, SLOT( restartUpdTimer( int ) ) );

    m_ui.pagesWidget->insertWidget( 0, &m_server );
    m_ui.pagesWidget->insertWidget( 1, &m_grid );
    m_ui.pagesWidget->insertWidget( 2, &m_workers );
    m_ui.pagesWidget->insertWidget( 3, &m_preferences );

    createIcons();
    m_ui.contentsWidget->setCurrentRow( m_CurrentPage );

    connect( m_grid.getJobSubmitter(), SIGNAL( changeNumberOfJobs( int ) ), &m_workers, SLOT( setNumberOfJobs( int ) ) );
}

CMainDlg::~CMainDlg()
{
    try
    {
        // Saving class to the config file
        _savecfg( *this, g_szCfgFileName );
    }
    catch ( ... )
    {
        QMessageBox::warning( this, "PROOFAgent Console",
                              "Can't save configuration to \
                              \"$GLITE_PROOF_LOCATION / etc / PAConsole.xml.cfg\"" );
    }
}

void CMainDlg::createIcons()
{
    QListWidgetItem *serverButton = new QListWidgetItem( m_ui.contentsWidget );
    serverButton->setIcon( QIcon( ":/images/server.png" ) );
    serverButton->setText( tr( "Server" ) );
    serverButton->setTextAlignment( Qt::AlignHCenter );
    serverButton->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    QListWidgetItem *gridButton = new QListWidgetItem( m_ui.contentsWidget );
    gridButton->setIcon( QIcon( ":/images/grid.png" ) );
    gridButton->setText( tr( "Grid" ) );
    gridButton->setTextAlignment( Qt::AlignHCenter );
    gridButton->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    QListWidgetItem *workersButton = new QListWidgetItem( m_ui.contentsWidget );
    workersButton->setIcon( QIcon( ":/images/workers.png" ) );
    workersButton->setText( tr( "Workers" ) );
    workersButton->setTextAlignment( Qt::AlignHCenter );
    workersButton->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    QListWidgetItem *preferencesButton = new QListWidgetItem( m_ui.contentsWidget );
    preferencesButton->setIcon( QIcon( ":/images/preferences.png" ) );
    preferencesButton->setText( tr( "Preferences" ) );
    preferencesButton->setTextAlignment( Qt::AlignHCenter );
    preferencesButton->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    connect( m_ui.contentsWidget,
             SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
             this, SLOT( changePage( QListWidgetItem *, QListWidgetItem* ) ) );
}

void CMainDlg::changePage( QListWidgetItem *_Current, QListWidgetItem *_Previous )
{
    if ( !_Current )
        _Current = _Previous;

    m_ui.pagesWidget->setCurrentIndex( m_ui.contentsWidget->row( _Current ) );
    m_CurrentPage = m_ui.pagesWidget->currentIndex();
}
