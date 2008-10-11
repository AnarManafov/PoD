/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-07-14
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef CLOGINFODLG_H_
#define CLOGINFODLG_H_

// Qt autogen. file
#include "ui_wgLogInfo.h"

class CLogInfoDlg: public QDialog
{
    Q_OBJECT

public:
    CLogInfoDlg( QWidget *_parent, const std::string &_gLiteJobID );
    virtual ~CLogInfoDlg();

public slots:
    int exec();
    void on_btnSave_clicked();

private:
    Ui::Dialog m_ui;
    std::string m_gLiteJobID;
};

#endif /* CLOGINFODLG_H_ */
