// Qt
#include <QApplication>

// Our
#include "MainDlg.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    CMainDlg dlg;
    dlg.show();
    return app.exec();
}
