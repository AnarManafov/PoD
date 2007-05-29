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

// STD
#include <sstream>

// Our
#include "MainDlg.h"
#include "ServerInfo.h"

using namespace std;


#define RUNNING_MESSAGE \
    Dialog::tr("<p>Message boxes have a caption, a text, " \
               "and any number of buttons, each with standard or custom texts." \
               "<p>Click a button to close the message box. Pressing the Esc button " \
               "will activate the detected escape button (if any).")


CMainDlg::CMainDlg(QDialog *parent)
        : QDialog(parent)
{
    ui.setupUi( this );
    // Show status on start-up
    on_btnStatusServer_clicked();
}

void CMainDlg::on_btnStatusServer_clicked()
{
    CServerInfo si;
    const pid_t pidXrootD = si.IsXROOTDRunning();
    const pid_t pidPA = si.IsPROOFAgentRunning();

    const QColor c = ( !pidXrootD || !pidPA ) ? QColor(255, 0, 0) : QColor(0, 0, 255);
    ui.edtServerInfo->setTextColor( c );

    stringstream ss;
    ss
    << si.GetXROOTDInfo() << "\n"
    << si.GetPAInfo() << "\n";

    ui.edtServerInfo->setText( QString(ss.str().c_str()) );
}

void CMainDlg::on_btnStartServer_clicked()
{
    CServerInfo si;
    pid_t pidXrootD = si.IsXROOTDRunning();
    pid_t pidPA = si.IsPROOFAgentRunning();
    if ( pidXrootD || pidPA )
    {
        QMessageBox::information(this, tr("PROOFAgent Console"), tr("<p>One or more components of the Server are running!<p> Please stop the Server first.") );
        return ;
    }

    system("./Server_gLitePROOF.sh start");
    pidXrootD = si.IsXROOTDRunning();
    pidPA = si.IsPROOFAgentRunning();
    if ( !pidXrootD || !pidPA )
    {
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while starting the Server!") );
        return ;
    }

    on_btnStatusServer_clicked();
}

void CMainDlg::on_btnStopServer_clicked()
{
    CServerInfo si;
    pid_t pidXrootD = si.IsXROOTDRunning();
    pid_t pidPA = si.IsPROOFAgentRunning();
    if ( !pidXrootD && !pidPA )
        return ;

    system("./Server_gLitePROOF.sh stop");
    pidXrootD = si.IsXROOTDRunning();
    pidPA = si.IsPROOFAgentRunning();
    if ( pidXrootD || pidPA )
    {
        QMessageBox::critical(this, tr("PROOFAgent Console"), tr("<p>An error occurred while stopping the Server!") );
        return ;
    }

    on_btnStatusServer_clicked();
}

void CMainDlg::on_btnBrowsePIDDir_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                        tr("Select pid directory of PROOFAgent"),
                        ui.edtPIDDir->text(),
                        QFileDialog::DontResolveSymlinks
                        | QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        ui.edtPIDDir->setText(directory);
}
