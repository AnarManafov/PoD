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

class CLogInfoDlg: public QWidget
{
        Q_OBJECT

    public:
        CLogInfoDlg( QWidget *_parent );
        virtual ~CLogInfoDlg();

    private:
        Ui::wgLogInfo m_ui;
};

#endif /* CLOGINFODLG_H_ */
