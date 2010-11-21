/************************************************************************/
/**
 * @file LogInfoDlg.cpp
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-07-14
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
// STD
#include <sstream>
// GAW
#include "gLiteAPIWrapper.h"
// Qt
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
// PAConsole
#include "LogInfoDlg.h"
//=============================================================================
using namespace std;
//=============================================================================
CLogInfoDlg::CLogInfoDlg( QWidget *_parent, const string &_gLiteJobID ) :
    QDialog( _parent ),
    m_gLiteJobID( _gLiteJobID )
{
    m_ui.setupUi( this );

}
//=============================================================================
CLogInfoDlg::~CLogInfoDlg()
{

}
//=============================================================================
int CLogInfoDlg::exec()
{
    // New title of the dialog
    setWindowTitle( QString( "Log Info for: " ) + m_gLiteJobID.c_str() );

    // Getting Logging information. Using GAW to do so.
    ostringstream ss;
    typedef glite_api_wrapper::CGLiteAPIWrapper GAW;
    try
    {
        GAW::Instance().GetJobManager().JobLogInfo( m_gLiteJobID, ss );
    }
    catch( const exception &_e )
    {
        QMessageBox::critical( this,
                               tr( "PROOFAgent Console" ),
                               tr( _e.what() ) );
    }

    m_ui.txtLogView->setPlainText( ss.str().c_str() );
    return QDialog::exec();
}
//=============================================================================
void CLogInfoDlg::on_btnSave_clicked()
{
    // Saving the logging info to a file
    const QString filename = QFileDialog::getSaveFileName( this, tr( "Save log info to a file" ) );
    QFile file( filename );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QMessageBox::critical( this,
                               tr( "PROOFAgent Console" ),
                               tr( "Can't write to the file \n\"%1\"\n" ).arg( filename ) );
        return;
    }
    QTextStream out( &file );
    out << "gLite job ID: " << m_gLiteJobID.c_str() << endl;
    out << m_ui.txtLogView->toPlainText() << endl;
}
