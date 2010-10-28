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

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef TREEITEMCONTAINER_H_
#define TREEITEMCONTAINER_H_
//=============================================================================
class CTreeItemContainer
{
        typedef std::pair<std::string, QTreeWidgetItem *> Item_t;
        typedef std::vector<Item_t> container_t;

    public:
        void update( const CJobSubmitter::jobslist_t &_Jobs, QTreeWidget *_Tree )
        {
            if( _Jobs != m_Jobs )
                _Reset( _Jobs, _Tree );
            else
                _Update();
        }

    private:
        void _Reset( const CJobSubmitter::jobslist_t &_Jobs, QTreeWidget *_Tree )
        {
            // Recreating the tree
            m_Jobs = _Jobs;
            m_Tree = _Tree;
            m_Tree->clear();
            m_ParentJobItem.clear();
            m_ParentJobItem.reserve( m_Jobs.size() );
            m_Children.clear();

            CJobSubmitter::jobslist_t::const_iterator iter = m_Jobs.begin();
            CJobSubmitter::jobslist_t::const_iterator iter_end = m_Jobs.end();
            for( ; iter != iter_end; ++iter )
            {
                QTreeWidgetItem *ParentJobItem = new QTreeWidgetItem( _Tree );
                m_ParentJobItem.push_back( container_t::value_type( *iter, ParentJobItem ) );
                ParentJobItem->setText( 0, iter->c_str() );
                ParentJobItem->setText( 1, getJobStatus( *iter ).c_str() );
                try
                {
                    MiscCommon::StringVector_t jobs;
                    MiscCommon::gLite::CJobStatusObj( *iter ).GetChildren( &jobs );
                    std::for_each( jobs.begin(), jobs.end(),
                                   boost::bind( boost::mem_fn( &CTreeItemContainer::addChildItem ), this, _1, ParentJobItem ) );
                }
                catch( const std::exception &_e )
                    {}
            }

            _Tree->setColumnWidth( 0, 260 );
            _Tree->expandAll();
        }

        void _Update()
        {
            // Updating the tree - statuses of items
            // loop over the parents
            for_each( m_ParentJobItem.begin(), m_ParentJobItem.end(),
                      boost::bind( boost::mem_fn( &CTreeItemContainer::updateItem ), this, _1 ) );
            // loop over the children if any
            for_each( m_Children.begin(), m_Children.end(),
                      boost::bind( boost::mem_fn( &CTreeItemContainer::updateItem ), this, _1 ) );
        }

        void updateItem( container_t::value_type &_Item )
        {
            if( !_Item.second )
                return;
            try
            {
                _Item.second->setText( 1, getJobStatus( _Item.first ).c_str() );
            }
            catch( const std::exception &_e )
                {}
        }

        void addChildItem( const std::string &_JobID, QTreeWidgetItem *_parent )
        {
            QTreeWidgetItem *item( new QTreeWidgetItem( _parent ) );
            item->setText( 0, _JobID.c_str() );
            item->setText( 1, getJobStatus( _JobID ).c_str() );
            m_Children.push_back( container_t::value_type( _JobID, item ) );
        }

        std::string getJobStatus( const std::string &_JobID )
        {
            std::string status;
            glite_api_wrapper::CGLiteAPIWrapper::Instance().GetJobManager().JobStatus( _JobID, &status );
            return status;
        }

    private:
        CJobSubmitter::jobslist_t m_Jobs;
        QTreeWidget *m_Tree;
        container_t m_ParentJobItem;
        container_t m_Children;
};

#endif /*TREEITEMCONTAINER_H_*/
