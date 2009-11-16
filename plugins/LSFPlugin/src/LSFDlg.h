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

        Copyright (c) 2008-2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFDLG_H_
#define LSFDLG_H_
//=============================================================================
// LSF plug-in
#include "LSFJobSubmitter.h"
// Qt autogen. file
#include "ui_wgLSF.h"
// PAConsole
#include "IJobManager.h"
//=============================================================================
class CJobInfoItemModel;
//=============================================================================
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

    protected:
        void showEvent( QShowEvent* );
        void hideEvent( QHideEvent* );

    signals:
        void changeNumberOfJobs( int _count );

    public slots:
        void on_btnSubmitClient_clicked();
        void recieveThreadMsg( const QString &_Msg );
        void setProgress( int _Val );
        void on_btnBrowseJobScript_clicked();
        void on_edtJobScriptFileName_textChanged( const QString & /*_text*/ );
        void setNumberOfJobs( int _count );
        void on_lsfQueueList_currentIndexChanged( int _index );

    private slots:
//        void copyJobID() const;
//        void cancelJob();
//        void getJobOutput();
//        void getJobLoggingInfo();
        void removeJob();
        void showContextMenu( const QPoint &_point );
        void expandTreeNode( const QModelIndex& );
        void collapseTreeNode( const QModelIndex& );

    private:
        void createActions();
        void UpdateAfterLoad();

        // serialization
        template<class Archive>
        void save( Archive & _ar, const unsigned int /*_version*/ ) const
        {
            _ar
            & BOOST_SERIALIZATION_NVP( m_JobScript )
            & BOOST_SERIALIZATION_NVP( m_WorkersCount )
            & BOOST_SERIALIZATION_NVP( m_JobSubmitter )
            & BOOST_SERIALIZATION_NVP( m_queue );
        }
        template<class Archive>
        void load( Archive & _ar, const unsigned int /*_version*/ )
        {
            _ar
            & BOOST_SERIALIZATION_NVP( m_JobScript )
            & BOOST_SERIALIZATION_NVP( m_WorkersCount )
            & BOOST_SERIALIZATION_NVP( m_JobSubmitter )
            & BOOST_SERIALIZATION_NVP( m_queue );
            UpdateAfterLoad();
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        Ui::wgGrid m_ui;
        QAction *copyJobIDAct;
        QAction *cancelJobAct;
        QAction *getJobOutputAct;
        QAction *getJobLoggingInfoAct;
        QAction *removeJobAct;
        QClipboard *clipboard;
        std::string m_JobScript;
        int m_AllJobsCount;
        int m_WorkersCount;
        CLSFJobSubmitter m_JobSubmitter;
        CJobInfoItemModel *m_treeModel;
        int m_updateInterval;
        std::string m_queue;
        QModelIndex m_expandedNode;
};

BOOST_CLASS_VERSION( CLSFDlg, 1 )

#endif /*LSFDLG_H_*/
