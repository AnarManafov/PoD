/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-01-09
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef TREEITEMCONTAINER_H_
#define TREEITEMCONTAINER_H_

// STD
#include <algorithm>
// BOOST
#include <boost/bind.hpp>

class CTreeItemContainer
{
        typedef std::pair<LS_LONG_INT_t, QTreeWidgetItem *> Item_t;
        typedef std::vector<Item_t> container_t;

    public:
        void update( const CLSFJobSubmitter::jobslist_t &_Jobs, QTreeWidget *_Tree )
        {
            if ( _Jobs != m_Jobs )
                _Reset( _Jobs, _Tree );
            else
                _Update();
        }

    private:
        void _Reset( const CLSFJobSubmitter::jobslist_t &_Jobs, QTreeWidget *_Tree )
        {
            // Recreating the tree
            m_Jobs = _Jobs;
            m_Tree = _Tree;
            m_Tree->clear();
            m_ParentJobItem.clear();
            m_ParentJobItem.reserve( m_Jobs.size() );
            m_Children.clear();

            CLsfMng lsf;
            lsf.init();
            CLSFJobSubmitter::jobslist_t::const_iterator iter = m_Jobs.begin();
            CLSFJobSubmitter::jobslist_t::const_iterator iter_end = m_Jobs.end();
            for ( ; iter != iter_end; ++iter )
            {
                QTreeWidgetItem *ParentJobItem = new QTreeWidgetItem( _Tree );
                m_ParentJobItem.push_back( container_t::value_type( *iter, ParentJobItem ) );
                std::ostringstream str;
                str << *iter;
                ParentJobItem->setText( 0, str.str().c_str() );
                ParentJobItem->setText( 1, getJobStatus( *iter ).c_str() );
                try
                {
                    CLsfMng::IDContainer_t jobs;
                    lsf.getChildren( *iter, &jobs );
                    std::for_each( jobs.begin(), jobs.end(),
                                   boost::bind( boost::mem_fn( &CTreeItemContainer::addChildItem ), this, _1, ParentJobItem ) );
                }
                catch ( const std::exception &_e )
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
            if ( !_Item.second )
                return;
            try
            {
                _Item.second->setText( 1, getJobStatus( _Item.first ).c_str() );
            }
            catch ( const std::exception &_e )
                {}
        }

        void addChildItem( LS_LONG_INT_t _JobID, QTreeWidgetItem *_parent )
        {
            QTreeWidgetItem *item( new QTreeWidgetItem( _parent ) );
            std::ostringstream str;
            str << _JobID;
            item->setText( 0, str.str().c_str() );
            item->setText( 1, getJobStatus( _JobID ).c_str() );
            m_Children.push_back( container_t::value_type( _JobID, item ) );
        }

        std::string getJobStatus( LS_LONG_INT_t _JobID )
        {
            CLsfMng lsf;
            lsf.init();
            std::string status = lsf.jobStatusString( _JobID );
            return status;
        }

    private:
        CLSFJobSubmitter::jobslist_t m_Jobs;
        QTreeWidget *m_Tree;
        container_t m_ParentJobItem;
        container_t m_Children;
};

#endif /*TREEITEMCONTAINER_H_*/
