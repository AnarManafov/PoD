/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-07-15
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
// QT
#include <QObject>
// PAConsole
#include "PreferencesDlg.h"
//=============================================================================
CPreferencesDlg::CPreferencesDlg( QWidget *_parent ):
        QWidget( _parent ),
        m_JobStatusUpdInterval( 15 ) // in seconds
{
    m_ui.setupUi( this );

    connect( m_ui.spinJobStatusUpd, SIGNAL( valueChanged( int ) ), this, SLOT( _changedJobStatusUpdInterval( int ) ) );
    UpdateAfterLoad();
}
//=============================================================================
CPreferencesDlg::~CPreferencesDlg()
{
}
//=============================================================================
void CPreferencesDlg::UpdateAfterLoad()
{
    m_ui.spinJobStatusUpd->setValue( m_JobStatusUpdInterval );
}
//=============================================================================
void CPreferencesDlg::_changedJobStatusUpdInterval( int _val )
{
    m_JobStatusUpdInterval = _val;
    emit changedJobStatusUpdInterval( _val );
}
