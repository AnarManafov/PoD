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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef CMAINDLG_H_
#define CMAINDLG_H_
//=============================================================================
// Qt autogen. file
#include "ui_maindlg.h"
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
// PAConsole
#include "ServerDlg.h"
#include "WorkersDlg.h"
#include "PreferencesDlg.h"
//=============================================================================
class IJobManager;
//=============================================================================
class CMainDlg: public QDialog
{
        Q_OBJECT

        friend class boost::serialization::access;
        typedef std::vector<IJobManager*> PluginVec_t;

    public:
        CMainDlg( QDialog *_Parent = NULL );
        virtual ~CMainDlg();

    signals:
        void numberOfJobs( int );

    public slots:
        void changePage( QListWidgetItem *current, QListWidgetItem *previous );
        void updatePluginTimer( int _interval );
        void changeWorkersUpdTimer( int _interval );
        void changeNumberOfJobs( int _count );

    private slots:
        void on_closeButton_clicked();
        void idleTimeout();

    protected:
        bool eventFilter( QObject *obj, QEvent *event );
        void childEvent( QChildEvent *e );
        void enumAllChildren( QObject* o );
        void switchAllSensors( bool _on );

    private:
        void createIcons();
        void loadPlugins();
        template<class Archive>
        void serialize( Archive &_ar, const unsigned int /*_file_version*/ )
        {
            _ar
            & BOOST_SERIALIZATION_NVP( m_CurrentPage )
            & BOOST_SERIALIZATION_NVP( m_preferences );
        }

    private:
        Ui::MainDlg m_ui;
        CWorkersDlg m_workers;
        CServerDlg m_server;
        CPreferencesDlg m_preferences;
        int m_CurrentPage;
        PluginVec_t m_plugins;
        QTimer *m_idleTimer;
};

BOOST_CLASS_VERSION( CMainDlg, 4 )

#endif /*CMAINDLG_H_*/
