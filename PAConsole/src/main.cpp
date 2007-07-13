/************************************************************************/
/**
 * @file main.cpp
 * @brief main file
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:   $LastChangedRevision: 831 $
        created by:          Anar Manafov
                                  2007-05-23
        last changed by:   $LastChangedBy: manafov $ $LastChangedDate: 2007-05-29 11:41:10 +0200 (Tue, 29 May 2007) $

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
// Qt
#include <QApplication>

// Our
#include "MainDlg.h"

int main(int argc, char **argv)
{
    QApplication app( argc, argv );
    CMainDlg dlg;
    dlg.show();
    return app.exec();
}
