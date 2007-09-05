/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-09-03
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef TREEITEMCONTAINER_H_
#define TREEITEMCONTAINER_H_

// GAW
#include "glite-api-wrapper/gLiteAPIWrapper.h"

class CTreeItemContainer
{
        typedef std::map<std::string, QTreeWidgetItem *> container_t;

    public:
        void update( const std::string &_ParentJobID, QTreeWidget *_Tree )
        {
            if ( _ParentJobID.empty() )
                return;

            if ( _ParentJobID != m_ParentJobID )
                _Reset( _ParentJobID, _Tree );
            else
                _Update();
        }

    private:
        void _Reset( const std::string &_ParentJobID, QTreeWidget *_Tree )
        {
            m_ParentJobID = _ParentJobID;
            m_Tree = _Tree;
            m_Tree->clear();
            m_Children.clear();

            m_ParentJobItem = new QTreeWidgetItem( _Tree );
            m_ParentJobItem->setText( 0, m_ParentJobID.c_str() );
            try
            {
                m_ParentJobItem->setText( 1, getJobStatus(m_ParentJobID).c_str() );
              
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(m_ParentJobID).GetChildren( &jobs );
                std::for_each( jobs.begin(), jobs.end(),
                               std::bind1st(std::mem_fun(&CTreeItemContainer::addItem), this) );

                _Tree->setColumnWidth( 0, 260 );
                _Tree->expandAll();
            }
            catch ( const std::exception &_e)
            {
            }
        }

        void _Update()
        {
            try
            {
                m_ParentJobItem->setText( 1, getJobStatus(m_ParentJobID).c_str() );
                
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(m_ParentJobID).GetChildren( &jobs );
                std::for_each( jobs.begin(), jobs.end(),
                               std::bind1st(std::mem_fun(&CTreeItemContainer::updateItem), this) );
            }
            catch ( const std::exception &_e)
            {
            }
        }

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

#endif /*TREEITEMCONTAINER_H_*/
