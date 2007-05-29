/************************************************************************/
/**
 * @file MainDlg.cpp
 * @brief Main dialog implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:   $LastChangedRevision$
        created by:          Anar Manafov
                                  2007-05-23
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/ 
// Qt
#include <QtGui>
#include <QtUiTools/QUiLoader>

// Our
#include "MainDlg.h"


CMainDlg::CMainDlg(QDialog *parent)
        : QDialog(parent)
{
    ui.setupUi( this );
}

void CMainDlg::on_btnStartServer_clicked()
{
    // ui.edtServerInfo->setTextColor(QColor(255, 0, 0));
    ui.edtServerInfo->setText(QString("Hello Test!"));
}
