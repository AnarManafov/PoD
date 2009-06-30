/************************************************************************/
/**
 * @file AlienDlg.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-06-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// PAConsole LSF plug-in
#include "AlienDlg.h"
// Qt
#include <QtGui>


CAlienDlg::CalienDlg( QWidget *parent ) :
        QWidget( parent )
{
    m_ui.setupUi( this );
}

CAlienDlg::~CAlienDlg()
{
}

QString CLSFDlg::getName() const
{
    return QString( "Alien\nJob Manager" );
}

QWidget* CLSFDlg::getWidget()
{
    return this;
}

QIcon CLSFDlg::getIcon()
{
    return QIcon( ":/images/alien.png" );
}

void CLSFDlg::startUpdTimer( int _JobStatusUpdInterval )
{
  // TODO: implement me!
}
int CLSFDlg::getJobsCount() const
{
  // TODO: implement me!
    return 0;
}

Q_EXPORT_PLUGIN2( AlienJobManager, CAlienDlg );
