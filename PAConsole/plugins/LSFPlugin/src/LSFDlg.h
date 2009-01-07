/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2008-12-09
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFDLG_H_
#define LSFDLG_H_

// LSF plug-in
#include "LSFJobSubmitter.h"
// Qt autogen. file
#include "ui_wgLSF.h"
// PAConsole
#include "IJobManager.h"


class CLSFDlg: public QWidget, IJobManager
{
        Q_OBJECT
        Q_INTERFACES( IJobManager )

        friend class boost::serialization::access;

    public:
        CLSFDlg( QWidget *parent = NULL );
        virtual ~CLSFDlg();

    public:
        // IJobManager interface
        QString getName() const;
        QWidget *getWidget();
        QIcon getIcon();
        void startUpdTimer( int _JobStatusUpdInterval );
        int getJobsCount() const;

        void setAllDefault();

    signals:
        void changeNumberOfJobs( int _count );

    public slots:
        void on_btnSubmitClient_clicked();
        void updateJobsTree();
        void recieveThreadMsg( const QString &_Msg );
        void setProgress( int _Val );
        void on_btnBrowseJobScript_clicked();
        void on_edtJobScriptFileName_textChanged( const QString & /*_text*/ );
        void setNumberOfJobs( int _count );
//
//
//    private slots:
//        void copyJobID() const;
//        void cancelJob();
//        void getJobOutput();
//        void getJobLoggingInfo();
//        void removeJob();
//
//    protected:
//        void contextMenuEvent( QContextMenuEvent *event );
//
    private:
//        void createActions();
        void UpdateAfterLoad();

        // serialization
        template<class Archive>
        void save( Archive & _ar, const unsigned int /*_version*/ ) const
        {
            _ar
            & BOOST_SERIALIZATION_NVP( m_JobScript )
            & BOOST_SERIALIZATION_NVP( m_JobsCount )
            & BOOST_SERIALIZATION_NVP( m_JobSubmitter );
        }
        template<class Archive>
        void load( Archive & _ar, const unsigned int /*_version*/ )
        {
            _ar
            & BOOST_SERIALIZATION_NVP( m_JobScript )
            & BOOST_SERIALIZATION_NVP( m_JobsCount )
            & BOOST_SERIALIZATION_NVP( m_JobSubmitter );
            UpdateAfterLoad();
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        Ui::wgGrid m_ui;
        QTimer *m_Timer;
        QAction *copyJobIDAct;
        QAction *cancelJobAct;
        QAction *getJobOutputAct;
        QAction *getJobLoggingInfoAct;
        QAction *removeJobAct;
        QClipboard *clipboard;
        std::string m_JobScript;
        int m_JobsCount;
        CLSFJobSubmitter m_JobSubmitter;
};

BOOST_CLASS_VERSION( CLSFDlg, 1 )

#endif /*LSFDLG_H_*/
