/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-06-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef ALIENDLG_H_
#define ALIENDLG_H_

// Qt autogen. file
#include "ui_wgAlien.h"
// PAConsole
#include "IJobManager.h"

class CLSFDlg: public QWidget, IJobManager
{
        Q_OBJECT
        Q_INTERFACES( IJobManager )

    public:
        CAlienDlg( QWidget *parent = NULL );
        virtual ~CAlienDlg();

    public:
        // IJobManager interface
        QString getName() const;
        QWidget *getWidget();
        QIcon getIcon();
        void startUpdTimer( int _JobStatusUpdInterval );
        int getJobsCount() const;

    signals:
        void changeNumberOfJobs( int _count );

    private:
        Ui::wgGrid m_ui;
};

#endif /*ALIENDLG_H_*/
