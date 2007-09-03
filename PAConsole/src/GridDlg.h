/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-08-24
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef GRIDDLG_H_
#define GRIDDLG_H_

// Qt autogen. file
#include "ui_wgGrid.h"

// MiscCommon
#include "def.h"

// PAConsole
#include "JobSubmitter.h"

typedef std::auto_ptr<CJobSubmitter> JobSubmitterPtr_t;

class CItemContainer
{
        typedef std::map<std::string, QTreeWidgetItem *> container_t;

    public:
        void Reset( const std::string &_ParentJobID, QTreeWidget *_Tree )
        {
            m_ParentJobID = _ParentJobID;
            m_Tree = _Tree;
            m_Tree->clear();
            m_Children.clear();

            m_ParentJobItem = new QTreeWidgetItem( _Tree );
            m_ParentJobItem->setText( 0, m_ParentJobID.c_str() );

            try
            {
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(m_ParentJobID).GetChildren( &jobs );
                std::for_each( jobs.begin(), jobs.end(),
                               std::bind1st(std::mem_fun(&CItemContainer::addItem), this) );

                _Tree->setColumnWidth( 0, 260 );
                _Tree->expandAll();
            }
            catch ( const std::exception &_e)
            {
            }
        }
        void Update()
        {
            try
            {
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(m_ParentJobID).GetChildren( &jobs );
                std::for_each( jobs.begin(), jobs.end(),
                               std::bind1st(std::mem_fun(&CItemContainer::updateItem), this) );
            }
            catch ( const std::exception &_e)
            {
            }
        }
        const std::string &GetParentJobID()
        {
            return m_ParentJobID;
        }

    private:
        void addItem ( std::string _JobID )
        {
            QTreeWidgetItem *item( new QTreeWidgetItem(m_ParentJobItem) );
            item->setText( 0, _JobID.c_str() );
            item->setText( 1, getJobStatus(_JobID).c_str() );
            m_Children.insert( std::make_pair(_JobID, item) );
        }

        void updateItem( std::string _JobID )
        {
            container_t::iterator map_iter = m_Children.find( _JobID );
            if ( m_Children.end() != map_iter )
                map_iter->second->setText( 1, getJobStatus(_JobID).c_str() );
            else
            {
                // TODO: add element or assert?
            }
        }
        
        std::string getJobStatus( const std::string &_JobID )
        {
          std::string status;
          glite_api_wrapper::CGLiteAPIWrapper::Instance().GetJobManager().JobStatus( _JobID, &status );
          return status;
        }

    private:
        std::string m_ParentJobID;
        QTreeWidget *m_Tree;
        QTreeWidgetItem *m_ParentJobItem;
        container_t m_Children;
};

class CGridDlg: public QWidget
{
        Q_OBJECT

    public:
        CGridDlg( QWidget *parent = 0 );
        virtual ~CGridDlg();

    public:
        CJobSubmitter *getJobSubmitter()
        {
            return m_JobSubmitter.get();
        }

    public slots:
        void on_btnSubmitClient_clicked();
        void updateJobsTree();
        void recieveThreadMsg( const QString &_Msg);
        void setProgress( int _Val );
        void on_btnBrowseJDL_clicked();

    private slots:
        void copyJobID() const;

    protected:
        void contextMenuEvent( QContextMenuEvent *event );

    private:
        void createActions();

    private:
        Ui::wgGrid m_ui;
        QTimer *m_Timer;
        JobSubmitterPtr_t m_JobSubmitter;
        QAction *copyJobIDAct;
        CItemContainer m_TreeItems;
        QClipboard *clipboard;
};

#endif /*GRIDDLG_H_*/
