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
// PAConsole
#include "WorkersDlg.h"

const char * const g_szPROOF_CFG = "$POD_LOCATION/etc/proofagent.cfg";

using namespace std;
using namespace MiscCommon;
using namespace boost;
using namespace boost::program_options;

void parsePROOFAgentCfgFile( string _cfgFileName, string *_retVal )
{
    smart_path( &_cfgFileName );

    string proofCfgFileName;
    variables_map vm;

    try
    {
        options_description config_file_options( "Configuration" );
        config_file_options.add_options()
        ( "general.proof_cfg_path", value<string>( &proofCfgFileName ), "" )

        // TODO: use allow_unregistered when BOOST 1.36+ can be used and the following options can be removed from here
        ( "general.isServerMode", value<bool>(), "" )
        ( "general.work_dir", value<string>(), "" )
        ( "general.logfile_dir", value<string>(), "" )
        ( "general.logfile_overwrite", value<bool>(), "" )
        ( "general.log_level", value<size_t>(), "" )
        ( "general.timeout", value<size_t>(), "" )
        ( "general.last_execute_cmd", value<string>(), "" )
        ( "server.listen_port", value<unsigned short>(), "" )
        ( "server.local_client_port_min", value<unsigned short>(), "" )
        ( "server.local_client_port_max", value<unsigned short>(), "" )
        ( "client.server_port", value<unsigned short>(), "" )
        ( "client.server_addr", value<string>(), "" )
        ( "client.local_proofd_port", value<unsigned short>(), "" )
        ( "client.shutdown_if_idle_for_sec", value<int>(), "" )
        ;

        // Load the file and tokenize it
        ifstream ifs( _cfgFileName.c_str() );
        if ( !ifs.good() )
        {
            // TODO: show an msg box
            cerr << "Could not open the PROOFAgent configuration file" << endl;
            return;
        }
        // Parse the config file
        // TODO: use allow_unregistered when BOOST 1.36+ can be used
        store( parse_config_file( ifs, config_file_options ), vm );
        notify( vm );
    }
    catch ( const exception &_e )
    {
        cerr << "Exception has been caught while reading PROOFAgent's configuration file: " << _e.what() << endl;
        return;
    }
    catch ( ... )
    {
        return;
    }

    *_retVal = proofCfgFileName;
}

CWorkersDlg::CWorkersDlg( QWidget *parent ):
        QWidget( parent ),
        m_bMonitorWorkers( true ),
        m_WorkersUpdInterval( 0 )
{
    m_ui.setupUi( this );

    parsePROOFAgentCfgFile( g_szPROOF_CFG, &m_CfgFileName );
    smart_path( &m_CfgFileName );
    if ( m_CfgFileName.empty() )
    {
        QMessageBox::critical( this, tr( "PROOFAgent Console" ), tr( "An Error occurred while retrieving proof.conf full name from proofagent.cfg" ) );
        //return ;
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
        m_Timer->start( m_WorkersUpdInterval );
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
    m_WorkersUpdInterval = _WorkersUpdInterval * 1000;
    if ( m_bMonitorWorkers > 0 )
        m_Timer->start( m_WorkersUpdInterval );
}
