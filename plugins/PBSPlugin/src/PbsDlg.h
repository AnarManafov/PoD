/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PBSDLG_H_
#define PBSDLG_H_
//=============================================================================
// Pbs plug-in
#include "PbsJobSubmitter.h"
// Qt autogen. file
#include "ui_wgPBS.h"
// PAConsole
#include "IJobManager.h"
namespace pbs_plug
{
//=============================================================================
    class CJobInfoItemModel;
//=============================================================================
    class CPbsDlg: public QWidget, IJobManager
    {
            Q_OBJECT
            Q_INTERFACES( IJobManager )

            friend class boost::serialization::access;

        public:
            CPbsDlg( QWidget *parent = NULL );
            virtual ~CPbsDlg();

        public:
            // IJobManager interface
            QString getName() const;
            QWidget *getWidget();
            QIcon getIcon();
            void startUpdTimer( int _JobStatusUpdInterval );
            void startUpdTimer( int _JobStatusUpdInterval, bool _hideMode );
            int getJobsCount() const;
            void setUserDefaults( const PoD::CPoDUserDefaults &_ud );

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
            void setNumberOfJobs( size_t _count );
            void on_queuesList_currentIndexChanged( int _index );

        private slots:
            void killJob();
            void removeJob();
            void removeAllCompletedJobs();
            void showContextMenu( const QPoint &_point );
            void expandTreeNode( const QModelIndex& );
            void collapseTreeNode( const QModelIndex& );
            void enableTree();

        private:
            void createActions();
            void UpdateAfterLoad();

            // serialization
            template<class Archive>
            void save( Archive & _ar, const unsigned int /*_version*/ ) const
            {
//            _ar
//            & BOOST_SERIALIZATION_NVP( m_JobScript )
//            & BOOST_SERIALIZATION_NVP( m_WorkersCount )
//            & BOOST_SERIALIZATION_NVP( m_JobSubmitter )
//            & BOOST_SERIALIZATION_NVP( m_queue );
            }
            template<class Archive>
            void load( Archive & _ar, const unsigned int /*_version*/ )
            {
//            _ar
//            & BOOST_SERIALIZATION_NVP( m_JobScript )
//            & BOOST_SERIALIZATION_NVP( m_WorkersCount )
//            & BOOST_SERIALIZATION_NVP( m_JobSubmitter )
//            & BOOST_SERIALIZATION_NVP( m_queue );
//            UpdateAfterLoad();
            }
            BOOST_SERIALIZATION_SPLIT_MEMBER()

        private:
            Ui::wgGrid m_ui;
            QAction *removeJobAct;
            QAction *removeAllCompletedJobsAct;
            QAction *killJobAct;
            QClipboard *clipboard;
            std::string m_JobScript;
            int m_AllJobsCount;
            int m_WorkersCount;
            CPbsJobSubmitter m_JobSubmitter;
            CJobInfoItemModel *m_treeModel;
            int m_updateInterval;
            std::string m_queue;
            QModelIndex m_expandedNode;
            std::string m_logDir;
            bool m_emailJobOutput;
    };
    
};

BOOST_CLASS_VERSION( pbs_plug::CPbsDlg, 1 )

#endif /*PBSDLG_H_*/
