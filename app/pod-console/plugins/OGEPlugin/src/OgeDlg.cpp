/************************************************************************/
/**
 * @file OgeDlg.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-10-13
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
// OGE plug-in
#include "JobInfoItemModel.h"
#include "OgeDlg.h"
// Qt
#include <QtGui>
// STD
#include <fstream>
// MiscCommon
#include "def.h"
#include "SysHelper.h"
// pod-console
#include "ServerInfo.h"
#include "version.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace oge_plug;
//=============================================================================
// making Qt to know this data type
// in order to use it in QVariant, for example
Q_DECLARE_METATYPE( oge_plug::SQueueInfo )
//=============================================================================
// default Job Script file
const LPCTSTR g_szDefaultJobScript = "$POD_LOCATION/etc/Job.oge";
// configuration file of the plug-in
const LPCTSTR g_szOgePluginCfgFileName = "$POD_LOCATION/etc/pod-console_OGE.xml.cfg";
//=============================================================================
// TODO: avoid of code duplications (this two function must be put in MiscCommon)
// Serialization helpers
template<class T>
void _loadcfg( T &_s, string _FileName )
{
    smart_path( &_FileName );
    if( _FileName.empty() || !does_file_exists( _FileName ) )
        throw exception();

    ifstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_iarchive ia( f );
    ia >> BOOST_SERIALIZATION_NVP( _s );
}
//=============================================================================
template<class T>
void _savecfg( const T &_s, string _FileName )
{
    smart_path( &_FileName );
    if( _FileName.empty() )
        throw exception();

    // make an archive
    ofstream f( _FileName.c_str() );
    //assert(f.good());
    boost::archive::xml_oarchive oa( f );
    oa << BOOST_SERIALIZATION_NVP( _s );
}
//=============================================================================
COgeDlg::COgeDlg( QWidget *parent ) :
    QWidget( parent ),
    m_AllJobsCount( 0 ),
    m_JobSubmitter( this ),
    m_updateInterval( 10000 ) // default value: 10 sec.
{
    m_ui.setupUi( this );

    connect( &m_JobSubmitter,
             SIGNAL( changeProgress( int ) ), this, SLOT( setProgress( int ) ) );
    connect( &m_JobSubmitter,
             SIGNAL( sendThreadMsg( const QString& ) ), this, SLOT( recieveThreadMsg( const QString& ) ) );

    createActions();

    clipboard = QApplication::clipboard();

    // Set completion for the edit box of JDL file name
    QCompleter *completer = new QCompleter( this );
    completer->setModel( new QDirModel( completer ) );
    m_ui.edtJobScriptFileName->setCompleter( completer );

    try
    {
        // Loading class from the config file
        _loadcfg( *this, g_szOgePluginCfgFileName );
    }
    catch( ... )
    {
        setAllDefault();
    }

    // Set the queues list
    try
    {
        COgeMng::queueInfoContainer_t queues;
        m_JobSubmitter.getQueues( &queues );
        COgeMng::queueInfoContainer_t::iterator iter = queues.begin();
        COgeMng::queueInfoContainer_t::iterator iter_end = queues.end();
        for( ; iter != iter_end; ++iter )
        {
            m_ui.queuesList->addItem( iter->m_name.c_str(), QVariant::fromValue( *iter ) );
            // selecting default
            if( m_queue.empty() )
            {
                // if there is no default queue set, then select any queue with the "proof" word in the name
                if( string::npos != iter->m_name.find( "proof" ) )
                    m_ui.queuesList->setCurrentIndex( distance( queues.begin(), iter ) );
            }
            else
            {
                if( iter->m_name == m_queue )
                    m_ui.queuesList->setCurrentIndex( distance( queues.begin(), iter ) );
            }
        }
    }
    catch( const exception &_e )
    {
        // TODO: handle it
    }

    // default queue name
    m_JobSubmitter.setQueue( m_queue );

    m_treeModel = new CJobInfoItemModel( &m_JobSubmitter, m_updateInterval );
    m_ui.treeJobs->setModel( m_treeModel );

    connect( m_treeModel, SIGNAL( doneUpdate() ), this, SLOT( enableTree() ) );


    connect( &m_JobSubmitter,
             SIGNAL( newJob( const COgeMng::jobID_t & ) ), m_treeModel, SLOT( addJob( const COgeMng::jobID_t & ) ) );
    connect( &m_JobSubmitter,
             SIGNAL( removedJob( const COgeMng::jobID_t & ) ), m_treeModel, SLOT( removeJob( const COgeMng::jobID_t & ) ) );
    connect( m_treeModel,
             SIGNAL( jobsCountUpdated( size_t ) ), this, SLOT( setNumberOfJobs( size_t ) ) );

    // a context menu of the table view
    m_ui.treeJobs->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( m_ui.treeJobs, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this, SLOT( showContextMenu( const QPoint & ) ) );

    connect( m_ui.treeJobs, SIGNAL( expanded( const QModelIndex& ) ),
             this, SLOT( expandTreeNode( const QModelIndex& ) ) );
    connect( m_ui.treeJobs, SIGNAL( collapsed( const QModelIndex& ) ),
             this, SLOT( collapseTreeNode( const QModelIndex& ) ) );
}
//=============================================================================
COgeDlg::~COgeDlg()
{
    try
    {
        // Saving class to the config file
        _savecfg( *this, g_szOgePluginCfgFileName );
    }
    catch( ... )
    {
    }

    delete m_treeModel;
}
//=============================================================================
void COgeDlg::setAllDefault()
{
    m_JobScript = g_szDefaultJobScript;
    m_WorkersCount = 1;
    m_JobSubmitter.setAllDefault();
    m_queue = "proof";
    UpdateAfterLoad();
}
//=============================================================================
void COgeDlg::UpdateAfterLoad()
{
    smart_path( &m_JobScript );
    m_ui.edtJobScriptFileName->setText( m_JobScript.c_str() );
    m_ui.spinNumWorkers->setValue( m_WorkersCount );
}
//=============================================================================
void COgeDlg::recieveThreadMsg( const QString &_Msg )
{
    QMessageBox::warning( this, tr( "pod-console" ), _Msg );
    m_ui.btnSubmitClient->setEnabled( true );
}
//=============================================================================
void COgeDlg::on_btnSubmitClient_clicked()
{
    // Checking queue up
    m_queue = m_ui.queuesList->currentText().toAscii().data();
    m_JobSubmitter.setQueue( m_queue );
    // Checking first that PoD server is running
    CServerInfo si;
    if( !si.IsRunning( true ) )
    {
        const string msg( "PoD server is not running.\n"
                          "Do you want to submit this job anyway?" );
        const QMessageBox::StandardButton reply =
            QMessageBox::question( this, tr( "pod-console" ), tr( msg.c_str() ),
                                   QMessageBox::Yes | QMessageBox::No );
        if( QMessageBox::Yes != reply )
            return;
    }

    if( !m_JobSubmitter.isRunning() )
    {
        if( !QFileInfo( m_ui.edtJobScriptFileName->text() ).exists() )
        {
            QMessageBox::critical( this,
                                   tr( "PROOFAgent Console" ),
                                   tr( "File\n\"%1\"\ndoesn't exist!" ).arg(
                                       m_ui.edtJobScriptFileName->text() ) );
            return;
        }

        m_JobScript = m_ui.edtJobScriptFileName->text().toAscii().data();

        m_WorkersCount = m_ui.spinNumWorkers->value();
        m_JobSubmitter.setNumberOfWorkers( m_WorkersCount );

        m_JobSubmitter.setJobScriptFilename( m_JobScript );

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
//=============================================================================
void COgeDlg::on_btnBrowseJobScript_clicked()
{
    const QString dir = QFileInfo( m_ui.edtJobScriptFileName->text() ).absolutePath();
    const QString filename = QFileDialog::getOpenFileName( this, tr( "Select a job script file" ), dir,
                                                           tr( "OGE script (*.pbs)" ) );
    if( QFileInfo( filename ).exists() )
    {
        m_JobScript = filename.toAscii().data();
        m_ui.edtJobScriptFileName->setText( filename );
    }
}
//=============================================================================
void COgeDlg::on_queuesList_currentIndexChanged( int _index )
{
    // max number of workers
    const QVariant data = m_ui.queuesList->itemData( _index );
    m_ui.spinNumWorkers->setMaximum( data.value<SQueueInfo>().m_maxJobs );
}
//=============================================================================
void COgeDlg::createActions()
{
    // Remove Job from the monitoring
    removeJobAct = new QAction( tr( "&Exclude" ), this );
    removeJobAct->setShortcut( tr( "Ctrl+E" ) );
    removeJobAct->setStatusTip( tr( "Remove the selected job from the monitoring" ) );
    // Remove all completed jobs from the monitoring
    removeAllCompletedJobsAct = new QAction( tr( "Re&move all completed" ), this );
    removeAllCompletedJobsAct->setShortcut( tr( "Ctrl+M" ) );
    removeAllCompletedJobsAct->setStatusTip( tr( "Remove all completed jobs from the monitoring" ) );
    // Kill LSF job
    killJobAct = new QAction( tr( "&Kill" ), this );
    killJobAct->setShortcut( tr( "Ctrl+K" ) );
    killJobAct->setStatusTip( tr( "Send a kill signal to the selected LSF job" ) );

    connect( removeJobAct, SIGNAL( triggered() ), this, SLOT( removeJob() ) );
    connect( removeAllCompletedJobsAct, SIGNAL( triggered() ), this, SLOT( removeAllCompletedJobs() ) );
    connect( killJobAct, SIGNAL( triggered() ), this, SLOT( killJob() ) );
}
//=============================================================================
void COgeDlg::showContextMenu( const QPoint &_point )
{
    // We need to disable menu items when no jobID is selected
    const QModelIndex clicked = m_ui.treeJobs->indexAt( _point );

    const bool enable( clicked.isValid() );

    QMenu menu( m_ui.treeJobs );
    menu.addAction( removeJobAct );
    removeJobAct->setEnabled( enable );
    menu.addAction( removeAllCompletedJobsAct );
    menu.addSeparator();
    menu.addAction( killJobAct );
    killJobAct->setEnabled( enable );

    menu.exec( QCursor::pos() );
}
//=============================================================================
void COgeDlg::expandTreeNode( const QModelIndex &_index )
{
    SJobInfo *info = reinterpret_cast< SJobInfo * >( _index.internalPointer() );
    if ( info )
        info->m_strStatus = "";
}
//=============================================================================
void COgeDlg::collapseTreeNode( const QModelIndex &_index )
{
    SJobInfo *info = reinterpret_cast< SJobInfo * >( _index.internalPointer() );
    if ( info )
        info->m_strStatus = "(expand to see the status)";
}
//=============================================================================
void COgeDlg::killJob()
{
    // TODO: if clicked on parent, than send kill to all its children

    // Job ID
    QModelIndex item = m_ui.treeJobs->currentIndex();
    if( !item.isValid() )
        return;

    SJobInfo *info = reinterpret_cast< SJobInfo * >( item.internalPointer() );
    if( !info )
        return;

    const string msg( "Are you sure you want to send a KILL signal to the selected job?\n"
                      "Be advised, after the signal is sent it will take some time until the job is killed and removed from the OGE queue." );
    const QMessageBox::StandardButton reply =
        QMessageBox::question( this, tr( PROJECT_NAME ), tr( msg.c_str() ),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    if( QMessageBox::Cancel == reply )
        return;

    try
    {
        m_JobSubmitter.killJob( info->m_id );
    }
    catch( const std::exception &_e )
    {
        QMessageBox::warning( this, tr( "pod-console" ), _e.what() );
    }
}
//=============================================================================
void COgeDlg::removeJob()
{
    // Job ID
    QModelIndex item = m_ui.treeJobs->currentIndex();
    if( !item.isValid() )
        return;

    SJobInfo *info = reinterpret_cast< SJobInfo * >( item.internalPointer() );
    if( !info )
        return;

    const string msg( "Are you sure you want to remove the selected job from the monitoring?\n"
                      "Be advised, removing the job from the monitoring will not kill/remove it from the OGE queue." );
    const QMessageBox::StandardButton reply =
        QMessageBox::question( this, tr( PROJECT_NAME ), tr( msg.c_str() ),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    if( QMessageBox::Cancel == reply )
        return;

    // if the m_id.empty(), then it means it is a rootItem - parent of
    // all parents
    m_JobSubmitter.removeJob(( NULL != info->parent() && !info->parent()->m_id.empty() ) ? info->parent()->m_id : info->m_id );
}
//=============================================================================
void COgeDlg::removeAllCompletedJobs()
{
    const string msg( "Are you sure you want to remove all completed jobs from the monitoring?\n"
                      "Be advised, removing a job from the monitoring will not kill/remove it from the OGE queue." );
    const QMessageBox::StandardButton reply =
        QMessageBox::question( this, tr( PROJECT_NAME ), tr( msg.c_str() ),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    if( QMessageBox::Cancel == reply )
        return;

    if( m_treeModel )
        m_treeModel->removeAllCompletedJobs();
}
//=============================================================================
void COgeDlg::enableTree()
{
    try
    {
        // Saving class to the config file
        _savecfg( *this, g_szOgePluginCfgFileName );
    }
    catch( ... )
    {
    }
}
//=============================================================================
void COgeDlg::setProgress( int _Val )
{
    if( 100 == _Val )
    {
        m_ui.btnSubmitClient->setEnabled( true );
    }
    m_ui.progressSubmittedJobs->setValue( _Val );
}
//=============================================================================
void COgeDlg::on_edtJobScriptFileName_textChanged( const QString &/*_text*/ )
{
    m_JobScript = m_ui.edtJobScriptFileName->text().toAscii().data();
}
//=============================================================================
void COgeDlg::setNumberOfJobs( size_t _count )
{
    m_AllJobsCount = _count;
    emit changeNumberOfJobs( _count );
}
//=============================================================================
QString COgeDlg::getName() const
{
    return QString( "OGE\nJob Manager" );
}
//=============================================================================
QWidget* COgeDlg::getWidget()
{
    return this;
}
//=============================================================================
QIcon COgeDlg::getIcon()
{
    return QIcon( ":/images/oge.png" );
}
//=============================================================================
void COgeDlg::startUpdTimer( int _JobStatusUpdInterval )
{
    if( _JobStatusUpdInterval <= 0 )
    {
        m_treeModel->setUpdateInterval( 0 );
        return;
    }
    // start or restart the timer
    if( _JobStatusUpdInterval > 0 )
    {
        m_updateInterval = _JobStatusUpdInterval;
        m_treeModel->setUpdateInterval( _JobStatusUpdInterval * 1000 );
    }
}
//=============================================================================
void COgeDlg::startUpdTimer( int _JobStatusUpdInterval, bool _hideMode )
{
    if( _hideMode )
    {
        // in hideMode we updated, but not very intensive (normal updated time +15 sec.).
        // this is needed for the GUI, to get a number of active jobs even when
        // plug-in is hidden
        m_treeModel->setUpdateInterval(( m_updateInterval + 15 ) * 1000 );
    }
}
//=============================================================================
void COgeDlg::showEvent( QShowEvent* )
{
    startUpdTimer( m_updateInterval );
}
//=============================================================================
void COgeDlg::hideEvent( QHideEvent* )
{
    startUpdTimer( 0, true );
}
//=============================================================================
int COgeDlg::getJobsCount() const
{
    return m_AllJobsCount;
}
//=============================================================================
void COgeDlg::setUserDefaults( const PoD::CPoDUserDefaults &_ud )
{
    m_JobSubmitter.setUserDefaults( _ud );
}
//=============================================================================
void COgeDlg::setEnvironment( const std::string &_envp )
{
    m_JobSubmitter.setEnvironment( _envp );
}

//=============================================================================
Q_EXPORT_PLUGIN2( OGEJobManager, COgeDlg );
