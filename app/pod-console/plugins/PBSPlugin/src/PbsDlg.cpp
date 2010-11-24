/************************************************************************/
/**
 * @file PbsDlg.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
// PBS plug-in
#include "JobInfoItemModel.h"
#include "PbsDlg.h"
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
using namespace pbs_plug;
//=============================================================================
// making Qt to know this data type
// in order to use it in QVariant, for example
Q_DECLARE_METATYPE( pbs_plug::SQueueInfo )
//=============================================================================
// default Job Script file
const LPCTSTR g_szDefaultJobScript = "$POD_LOCATION/etc/Job.pbs";
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
CPbsDlg::CPbsDlg( QWidget *parent ) :
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
}
//=============================================================================
CPbsDlg::~CPbsDlg()
{
    try
    {
        // Saving class to the config file
        _savecfg( *this, m_configFile );
    }
    catch( ... )
    {
    }

    delete m_treeModel;
}
//=============================================================================
void CPbsDlg::setAllDefault()
{
    m_JobScript = g_szDefaultJobScript;
    m_WorkersCount = 1;
    m_JobSubmitter.setAllDefault();
    m_queue = "proof";
    UpdateAfterLoad();
}
//=============================================================================
void CPbsDlg::UpdateAfterLoad()
{
    smart_path( &m_JobScript );
    m_ui.edtJobScriptFileName->setText( m_JobScript.c_str() );
    m_ui.spinNumWorkers->setValue( m_WorkersCount );
}
//=============================================================================
void CPbsDlg::recieveThreadMsg( const QString &_Msg )
{
    QMessageBox::warning( this, tr( "pod-console" ), _Msg );
    m_ui.btnSubmitClient->setEnabled( true );
}
//=============================================================================
void CPbsDlg::on_btnSubmitClient_clicked()
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
void CPbsDlg::on_btnBrowseJobScript_clicked()
{
    const QString dir = QFileInfo( m_ui.edtJobScriptFileName->text() ).absolutePath();
    const QString filename = QFileDialog::getOpenFileName( this, tr( "Select a job script file" ), dir,
                                                           tr( "PBS script (*.pbs)" ) );
    if( QFileInfo( filename ).exists() )
    {
        m_JobScript = filename.toAscii().data();
        m_ui.edtJobScriptFileName->setText( filename );
    }
}
//=============================================================================
void CPbsDlg::on_queuesList_currentIndexChanged( int _index )
{
    // max number of workers
    const QVariant data = m_ui.queuesList->itemData( _index );
    m_ui.spinNumWorkers->setMaximum( data.value<SQueueInfo>().m_maxJobs );
}
//=============================================================================
void CPbsDlg::createActions()
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
void CPbsDlg::showContextMenu( const QPoint &_point )
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
void CPbsDlg::expandTreeNode( const QModelIndex &_index )
{
//    // expand only one node at time to reduce a number of requests to LSF daemon
//    if ( m_expandedNode.isValid() && m_expandedNode != _index )
//        m_ui.treeJobs->collapse( m_expandedNode );
//
//
//    SJobInfo *info = reinterpret_cast< SJobInfo * >( _index.internalPointer() );
//    if ( info )
//        info->m_expanded = true;
//
//    m_expandedNode = _index;
}
//=============================================================================
void CPbsDlg::collapseTreeNode( const QModelIndex &_index )
{
//    SJobInfo *info = reinterpret_cast< SJobInfo * >( _index.internalPointer() );
//    if ( info )
//        info->m_expanded = false;
}
//=============================================================================
void CPbsDlg::killJob()
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
                      "Be advised, after the signal is sent it will take some time until the job is killed and removed from the PBS queue." );
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
void CPbsDlg::removeJob()
{
    // Job ID
    QModelIndex item = m_ui.treeJobs->currentIndex();
    if( !item.isValid() )
        return;

    SJobInfo *info = reinterpret_cast< SJobInfo * >( item.internalPointer() );
    if( !info )
        return;

    const string msg( "Are you sure you want to remove the selected job from the monitoring?\n"
                      "Be advised, removing the job from the monitoring will not kill/remove it from the PBS queue." );
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
void CPbsDlg::removeAllCompletedJobs()
{
    const string msg( "Are you sure you want to remove all completed jobs from the monitoring?\n"
                      "Be advised, removing a job from the monitoring will not kill/remove it from the PBS queue." );
    const QMessageBox::StandardButton reply =
        QMessageBox::question( this, tr( PROJECT_NAME ), tr( msg.c_str() ),
                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    if( QMessageBox::Cancel == reply )
        return;

    if( m_treeModel )
        m_treeModel->removeAllCompletedJobs();
}
//=============================================================================
void CPbsDlg::enableTree()
{
    try
    {
        // Saving class to the config file
        _savecfg( *this, m_configFile );
    }
    catch( ... )
    {
    }
}
//=============================================================================
void CPbsDlg::setProgress( int _Val )
{
    if( 100 == _Val )
    {
        m_ui.btnSubmitClient->setEnabled( true );
    }
    m_ui.progressSubmittedJobs->setValue( _Val );
}
//=============================================================================
void CPbsDlg::on_edtJobScriptFileName_textChanged( const QString &/*_text*/ )
{
    m_JobScript = m_ui.edtJobScriptFileName->text().toAscii().data();
}
//=============================================================================
void CPbsDlg::setNumberOfJobs( size_t _count )
{
    m_AllJobsCount = _count;
    emit changeNumberOfJobs( _count );
}
//=============================================================================
QString CPbsDlg::getName() const
{
    return QString( "PBS\nJob Manager" );
}
//=============================================================================
QWidget* CPbsDlg::getWidget()
{
    return this;
}
//=============================================================================
QIcon CPbsDlg::getIcon()
{
    return QIcon( ":/images/pbs.png" );
}
//=============================================================================
void CPbsDlg::startUpdTimer( int _JobStatusUpdInterval )
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
void CPbsDlg::startUpdTimer( int _JobStatusUpdInterval, bool _hideMode )
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
void CPbsDlg::showEvent( QShowEvent* )
{
    startUpdTimer( m_updateInterval );
}
//=============================================================================
void CPbsDlg::hideEvent( QHideEvent* )
{
    startUpdTimer( 0, true );
}
//=============================================================================
int CPbsDlg::getJobsCount() const
{
    return m_AllJobsCount;
}
//=============================================================================
void CPbsDlg::setUserDefaults( const PoD::CPoDUserDefaults &_ud )
{
    m_JobSubmitter.setUserDefaults( _ud );

    m_configFile = _ud.getOptions().m_server.m_common.m_workDir;
    smart_append( &m_configFile, '/' );
    m_configFile += "etc/pod-console_PBS.xml.cfg";
    smart_path( &m_configFile );

    try
    {
        // Loading class from the config file
        _loadcfg( *this, m_configFile );
    }
    catch( ... )
    {
        setAllDefault();
    }

    // Set the queues list
    try
    {
        CPbsMng::queueInfoContainer_t queues;
        m_JobSubmitter.getQueues( &queues );
        CPbsMng::queueInfoContainer_t::iterator iter = queues.begin();
        CPbsMng::queueInfoContainer_t::iterator iter_end = queues.end();
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
             SIGNAL( newJob( const CPbsMng::jobID_t & ) ), m_treeModel, SLOT( addJob( const CPbsMng::jobID_t & ) ) );
    connect( &m_JobSubmitter,
             SIGNAL( removedJob( const CPbsMng::jobID_t & ) ), m_treeModel, SLOT( removeJob( const CPbsMng::jobID_t & ) ) );
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
void CPbsDlg::setEnvironment( const std::string &_envp )
{
    m_JobSubmitter.setEnvironment( _envp );
}

//=============================================================================
Q_EXPORT_PLUGIN2( PBSJobManager, CPbsDlg );
