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
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
// PAConsole
#include "ServerDlg.h"
#include "GridDlg.h"
#include "WorkersDlg.h"


class CMainDlg: public QDialog
{
        Q_OBJECT

        friend class boost::serialization::access;

    public:
        CMainDlg( QDialog *_Parent = 0 );
        virtual ~CMainDlg();

    public slots:
        void changePage(QListWidgetItem *current, QListWidgetItem *previous);

    private:
        void createIcons();
        template<class Archive>
        void serialize(Archive &_ar, const unsigned int /*_file_version*/)
        {
            _ar
            //& BOOST_SERIALIZATION_NVP(m_grid)
            //& BOOST_SERIALIZATION_NVP(m_workers)
            & BOOST_SERIALIZATION_NVP(m_server);
        }

    private:
        Ui::MainDlg m_ui;

        CGridDlg m_grid;
        CWorkersDlg m_workers;
        CServerDlg m_server;
};

BOOST_CLASS_VERSION(CMainDlg, 1)

#endif /*CMAINDLG_H_*/
