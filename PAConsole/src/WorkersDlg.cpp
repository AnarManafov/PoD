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
#include <QtXml/QDomDocument>
#include <QMessageBox>
#include <QFile>

// STD
#include <fstream>

// MiscCommon
#include "CustomIterator.h"
#include "SysHelper.h"

// PAConsole
#include "WorkersDlg.h"

const size_t g_TimeoutCheckPROOFCONF = 5000;

using namespace std;
using namespace MiscCommon;

CWorkersDlg::CWorkersDlg( QWidget *parent ): QWidget( parent )
{
    m_ui.setupUi( this );

    getPROOFCfg( &m_CfgFileName );
    smart_homedir_append( &m_CfgFileName );
    if ( m_CfgFileName.empty() )
    {
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("An Error occurred while retrieving proof.conf full name from proofagent.cfg.xml") );
        //return ;
    }

    m_Timer = new QTimer(this);
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(update()) );

    on_chkShowWorkers_stateChanged( Qt::Checked );

    setActiveWorkers( 0 );
}

CWorkersDlg::~CWorkersDlg()
{
    if ( m_Timer )
    {
        m_Timer->stop();
        delete m_Timer;
    }
}

/// Simple method to read full name of the proof.conf from proofagent.cfg.xml (cfg file is hard-coded so far)
// TODO: Revise method and remove hard-coded reference to proofagent.cfg.xml
void CWorkersDlg::getPROOFCfg( string *_FileName )
{
    if ( !_FileName )
        return ;

    QFile file( tr("./proofagent.cfg.xml") );
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("PROOFAgent Console"),
                             tr("Cannot read file %1:\n%2.")
                             .arg("./proofagent.cfg.xml")
                             .arg(file.errorString()));
        return ;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if ( !domDocument.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
    {
        QMessageBox::information(window(), tr("PROOFAgent Console"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return ;
    }
    QDomNodeList server = domDocument.elementsByTagName( tr("server") );
    QDomNode node = server.at(0).namedItem( tr("config") );
    if ( node.isNull())
        return ; // TODO: Msg me!

    QDomNode cfg = node.attributes().namedItem( tr("proof_cfg_path") );
    if ( cfg.isNull())
        return ; // TODO: Msg me!

    *_FileName = cfg.nodeValue().toAscii().data();
}

void CWorkersDlg::update()
{
    // Read proof.conf and update Listbox
    ifstream f( m_CfgFileName.c_str() );
    if ( !f.is_open() )
        return ;

    StringVector_t vec;

    copy(custom_istream_iterator<string>(f),
         custom_istream_iterator<string>(),
         back_inserter(vec));

    const int cur_sel = m_ui.lstClientsList->currentRow();
    m_ui.lstClientsList->clear();

    // Reading only comment blocks of proof.conf
    const char chCmntSign('#');
    StringVector_t::iterator iter = find_if( vec.begin(), vec.end(),
                                          SFindComment<string>(chCmntSign) );
    StringVector_t::const_iterator iter_end = vec.end();
    while ( iter != iter_end )
    {
        trim_left( &*iter, chCmntSign );
        m_ui.lstClientsList->addItem( iter->c_str() );
        iter = find_if( ++iter, vec.end(),
                        SFindComment<string>(chCmntSign) );
    }
    if ( m_ui.lstClientsList->count() >= cur_sel )
        m_ui.lstClientsList->setCurrentRow( cur_sel );

    setActiveWorkers( m_ui.lstClientsList->count() - 1 );
}

void CWorkersDlg::on_chkShowWorkers_stateChanged( int _Stat )
{
    const bool bEnable = (_Stat == Qt::Checked);
    if ( m_Timer->isActive() && bEnable )
        return ; // TODO: need assert here

    m_ui.lstClientsList->setEnabled( bEnable );

    if ( !bEnable )
        m_Timer->stop();
    else
        m_Timer->start(g_TimeoutCheckPROOFCONF);
}
