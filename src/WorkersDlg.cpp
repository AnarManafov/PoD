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

        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// Qt
#include <QWidget>
#include <QMessageBox>
#include <QFile>
#include <QTimer>
// STD
#include <fstream>
// BOOST
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// MiscCommon
#include "CustomIterator.h"
#include "SysHelper.h"
#include "PoDUserDefaultsOptions.h"
// PAConsole
#include "WorkersDlg.h"
#include "version.h"

const char * const g_szPoDcfg = "$POD_LOCATION/etc/PoD.cfg";

using namespace std;
using namespace MiscCommon;
using namespace boost;
using namespace boost::program_options;

void parsePROOFAgentCfgFile( string _cfgFileName, string *_retVal )
{
    smart_path( &_cfgFileName );

    PoD::CPoDUserDefaults user_defaults;
    user_defaults.init( _cfgFileName );

    *_retVal = user_defaults.getValueForKey( "server.proof_cfg_path" );
}

CWorkersDlg::CWorkersDlg( QWidget *parent ):
        QWidget( parent ),
        m_bMonitorWorkers( true ),
        m_WorkersUpdInterval( 0 )
{
    m_ui.setupUi( this );

    try
    {
        parsePROOFAgentCfgFile( g_szPoDcfg, &m_CfgFileName );
    }
    catch ( exception &e )
    {
        QMessageBox::critical( this,
                               QString( PROJECT_NAME ),
                               tr( e.what() ) );
        // TODO: implement a graceful quit
        exit( 1 );
    }
    smart_path( &m_CfgFileName );
    if ( m_CfgFileName.empty() )
    {
        QMessageBox::critical( this,
                               tr( PROJECT_NAME ),
                               tr( "An Error occurred while retrieving a proof.conf location.\nPlease, check PoD configuration file." ) );
    }

    m_Timer = new QTimer( this );
    connect( m_Timer, SIGNAL( timeout() ), this, SLOT( update() ) );

    on_chkShowWorkers_stateChanged( m_bMonitorWorkers ? Qt::Checked : Qt::Unchecked );

    setActiveWorkers( 0 );
}

CWorkersDlg::~CWorkersDlg()
{
}

int CWorkersDlg::getWorkersFromPROOFCfg()
{
    // Read proof.conf and update Listbox
    ifstream f( m_CfgFileName.c_str() );
    if ( !f.is_open() )
    {
        m_ui.lstClientsList->clear();
        return 0;
    }

    StringVector_t vec;

    copy( custom_istream_iterator<string>( f ),
          custom_istream_iterator<string>(),
          back_inserter( vec ) );

    const int cur_sel = m_ui.lstClientsList->currentRow();
    m_ui.lstClientsList->clear();

    // Reading only comment blocks of proof.conf
    const char chCmntSign( '#' );
    StringVector_t::iterator iter = find_if( vec.begin(), vec.end(),
                                             SFindComment<string>( chCmntSign ) );
    StringVector_t::const_iterator iter_end = vec.end();
    while ( iter != iter_end )
    {
        trim_left<string>( &*iter, chCmntSign );
        m_ui.lstClientsList->addItem( iter->c_str() );
        iter = find_if( ++iter, vec.end(),
                        SFindComment<string>( chCmntSign ) );
    }
    if ( m_ui.lstClientsList->count() >= cur_sel )
        m_ui.lstClientsList->setCurrentRow( cur_sel );

    return ( m_ui.lstClientsList->count() - 1 );
}

void CWorkersDlg::update()
{
    // Don't process if the page is hidden
    // if ( isHidden() )
    //     return;

    setActiveWorkers( getWorkersFromPROOFCfg() );
}

void CWorkersDlg::on_chkShowWorkers_stateChanged( int _Stat )
{
    m_bMonitorWorkers = ( _Stat == Qt::Checked );
    if ( m_Timer->isActive() && m_bMonitorWorkers )
        return ; // TODO: need an assert here

    m_ui.lstClientsList->setEnabled( m_bMonitorWorkers );

    if ( !m_bMonitorWorkers )
        m_Timer->stop();
    else if ( m_WorkersUpdInterval > 0 )
    	restartUpdTimer( m_WorkersUpdInterval );
}

void CWorkersDlg::setActiveWorkers( size_t _Val1, size_t _Val2 )
{
    static size_t nTotal = 0;
    if ( _Val2 )
        nTotal = _Val2;
    tstring strMsg( _T( "Monitor connections (available %1 out of %2 worker(s)):" ) );
    tstringstream ss;
    ss << _Val1;
    replace<tstring>( &strMsg, _T( "%1" ), ss.str() );
    ss.str( "" );
    ss << nTotal;
    replace<tstring>( &strMsg, _T( "%2" ), ss.str() );
    m_ui.chkShowWorkers->setText( strMsg.c_str() );
}

void CWorkersDlg::restartUpdTimer( int _WorkersUpdInterval )
{
    if ( _WorkersUpdInterval <= 0 )
    {
        m_Timer->stop();
        return;
    }

    // convert to milliseconds
    m_WorkersUpdInterval = _WorkersUpdInterval;
    m_Timer->start( m_WorkersUpdInterval * 1000 );
}


void CWorkersDlg::showEvent( QShowEvent* )
{
    update();
    restartUpdTimer( m_WorkersUpdInterval );
}

void CWorkersDlg::hideEvent( QHideEvent* )
{
    m_Timer->stop();
}
