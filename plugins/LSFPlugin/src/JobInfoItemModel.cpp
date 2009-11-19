/************************************************************************/
/**
 * @file JobInfoItemModel.cpp
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-08
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
// STD
#include <sstream>
// BOOST
#include <boost/bind.hpp>
// vnetstat
#include "JobInfoItemModel.h"
//=============================================================================
using namespace std;
//=============================================================================
CJobInfoItemModel::CJobInfoItemModel( const CLSFJobSubmitter *_lsfsubmitter, int _updateInterval, QObject * _parent ):
        QAbstractItemModel( _parent ),
        m_jobinfo( _lsfsubmitter ),
        m_updateInterval( _updateInterval )
{
    m_rootItem = new SJobInfo();

    _setupHeader();
    _setupJobsContainer();
}
//=============================================================================
CJobInfoItemModel::~CJobInfoItemModel()
{
    delete m_rootItem;
}
//=============================================================================
int CJobInfoItemModel::rowCount( const QModelIndex &_parent ) const
{
    SJobInfo *parentItem = NULL;

    if ( !_parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = reinterpret_cast<SJobInfo *>( _parent.internalPointer() );

    return parentItem->m_children.size();
}
//=============================================================================
int CJobInfoItemModel::columnCount( const QModelIndex & parent ) const
{
    return m_Titles.count();
}
//=============================================================================
QVariant CJobInfoItemModel::data( const QModelIndex &_index, int _role ) const
{
    // This function is called a lot of times and it therefore needs to be very optimized

    if ( !_index.isValid() )
        return QVariant();
    if ( _index.column() >= m_Titles.count() )
        return QVariant();

    ostringstream ss;
    SJobInfo *info = reinterpret_cast< SJobInfo * >( _index.internalPointer() );
    switch ( _role )
    {
        case Qt::DisplayRole:
            {
                switch ( _index.column() )
                {
                    case TitleJobID:
                        return QString( info->m_strID.c_str() );
                    case TitleJobStatus:
                        return QString( info->m_strStatus.c_str() );
                    default:
                        return QVariant();
                }
                break;
            default:
                return QVariant();
            }
    }
    return QVariant();
}
//=============================================================================
QVariant CJobInfoItemModel::headerData( int _section, Qt::Orientation _orientation, int _role ) const
{
    if ( _role != Qt::DisplayRole )
        return QVariant();
    if ( _orientation != Qt::Horizontal )
        return QVariant();
    if ( _section < 0 || _section >= m_Titles.count() )
        return QVariant();

    return m_Titles[_section];
}
//=============================================================================
Qt::ItemFlags CJobInfoItemModel::flags( const QModelIndex & _index ) const
{
    if ( !_index.isValid() )
        return 0;

    return QAbstractItemModel::flags( _index ) & ( ~Qt::ItemIsEditable );
}
//=============================================================================
QModelIndex CJobInfoItemModel::index( int _row, int _column, const QModelIndex & _parent ) const
{
    if ( _row < 0 )
        return QModelIndex();
    if ( _column < 0 || _column >= m_Titles.count() )
        return QModelIndex();

    SJobInfo *parentItem = NULL;

    if ( !_parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = reinterpret_cast<SJobInfo *>( _parent.internalPointer() );

    if ( _row >= static_cast<int>( parentItem->m_children.size() ) )
        return QModelIndex();

    SJobInfo *childItem = parentItem->m_children[_row].get();
    if ( childItem )
        return createIndex( _row, _column, childItem );
    else
        return QModelIndex();
}
//=============================================================================
QModelIndex CJobInfoItemModel::parent( const QModelIndex & _index ) const
{
    if ( !_index.isValid() )
        return QModelIndex();

    SJobInfo *childItem = reinterpret_cast<SJobInfo *>( _index.internalPointer() );
    SJobInfo *parentItem = childItem->m_parent;

    if ( parentItem == m_rootItem )
        return QModelIndex();

    return createIndex( parentItem->row(), 0, parentItem );
}
//=============================================================================
void CJobInfoItemModel::_setupHeader()
{
    QStringList titles;
    titles
    << "ID"
    << "Status";

    beginInsertColumns( QModelIndex(), 0, titles.count() - 1 );
    m_Titles = titles;
    endInsertColumns();
}
//=============================================================================
void CJobInfoItemModel::jobChanged( SJobInfo *_info )
{
    if ( !_info )
        return;

    const size_t row = _info->row();
    if ( row < 0 )
        return;

    QModelIndex startIndex = createIndex( row, 0, _info );
    QModelIndex endIndex = createIndex( row, m_Titles.count(), _info );
    emit dataChanged( startIndex, endIndex );
}
//=============================================================================
void CJobInfoItemModel::beginInsertRow( const SJobInfoPTR_t &_info )
{
    if ( !_info->m_parent )
    {
        const int row = m_rootItem->m_children.size();

        beginInsertRows( QModelIndex(), row, row );
        _info->m_parent = m_rootItem;
        m_rootItem->addChild( _info );
        endInsertRows();
        return;
    }

    const int row = _info->m_parent->m_children.size();
    QModelIndex parentModelIndex = createIndex( row, 0, _info->m_parent );

    beginInsertRows( parentModelIndex, row, row );
    endInsertRows();

    emit doneUpdate();
}
//=============================================================================
void CJobInfoItemModel::beginRemoveRow( const SJobInfoPTR_t &_info )
{
    // mutex.lock();

    size_t row = 0;
    if ( _info->m_parent == m_rootItem )
    { // it is a parent job
        row = _info->row();
        emit beginRemoveRows( QModelIndex(), row, row );
        // removing the item from the root item children list
        m_rootItem->m_children.erase( remove( m_rootItem->m_children.begin(), m_rootItem->m_children.end(), _info ),
                                      m_rootItem->m_children.end() );
    }
    else
    { // its one of the children
        row = _info->row();
        QModelIndex idx = createIndex( row, 0, _info->m_parent );
        emit beginRemoveRows( idx, row, row );
    }

    //   g_signalIsPosted.wakeAll();
    //  mutex.unlock();

    endRemoveRows();

    emit doneUpdate();
}
//=============================================================================
void CJobInfoItemModel::_setupJobsContainer()
{
    connect( &m_jobinfo, SIGNAL( jobChanged( SJobInfo * ) ), this, SLOT( jobChanged( SJobInfo * ) ) );
    connect( &m_jobinfo, SIGNAL( addJob( const SJobInfoPTR_t& ) ), this, SLOT( beginInsertRow( const SJobInfoPTR_t& ) ) );
    connect( &m_jobinfo, SIGNAL( removeJob( const SJobInfoPTR_t& ) ), this, SLOT( beginRemoveRow( const SJobInfoPTR_t& ) ) );

    m_jobinfo.update( m_updateInterval );
}
//=============================================================================
void CJobInfoItemModel::setUpdateInterval( int _newVal )
{
    m_updateInterval = _newVal;
    _newVal <= 0 ? m_jobinfo.stopUpdate() : m_jobinfo.update( m_updateInterval );
}
//=============================================================================
void CJobInfoItemModel::numberOfJobsChanged( int _count )
{
    m_jobinfo.updateNumberOfJobs();
}
