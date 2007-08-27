/************************************************************************/
/**
 * @file MainDlg.h
 * @brief Main dialog declaration
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-05-23
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef CMAINDLG_H_
#define CMAINDLG_H_

// Qt autogen. file
#include "ui_maindlg.h"

class CMainDlg: public QDialog
{
        Q_OBJECT

    public:
        CMainDlg( QDialog *_Parent = 0 );
        ~CMainDlg()
        {}

    public slots:
        void changePage(QListWidgetItem *current, QListWidgetItem *previous);

    private:
        void createIcons();

    private:
        Ui::MainDlg m_ui;
};

#endif /*CMAINDLG_H_*/
