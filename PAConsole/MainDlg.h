#ifndef CMAINDLG_H_
#define CMAINDLG_H_

#include "ui_maindlg.h"

class CMainDlg : public QDialog
{
    Q_OBJECT

public:
    CMainDlg( QDialog *parent = 0 );

private:
    Ui::MainDlg ui;
};

#endif /*CMAINDLG_H_*/
