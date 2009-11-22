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
        m_updateInterval( _updateInterval ),
        m_rootItem( new SJobInfo( 0 ) )
{
    _setupHeader();
    _setupJobsContainer();
}
//=============================================================================
CJobInfoItemModel::~CJobInfoItemModel()
{
    delete m_rootItem;
}
//=============================================================================
void CJobInfoItemModel::_setupJobsContainer()
{
    connect( &m_jobinfo, SIGNAL( jobChanged( SJobInfo* ) ), this, SLOT( jobChanged( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( addJob( SJobInfo* ) ), this, SLOT( beginInsertRow( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( removeJob( SJobInfo* ) ), this, SLOT( beginRemoveRow( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( numberOfActiveJobsChanged( size_t ) ), this, SLOT( numberOfActiveJobsChanged( size_t ) ) );

    m_jobinfo.update( m_updateInterval );
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

    return ( QAbstractItemModel::flags( _index ) & ( ~Qt::ItemIsEditable ) );
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

    Q_ASSERT( parentItem );

    int chld_size = static_cast<int>( parentItem->m_children.size() );
    if ( _row >= chld_size || _row < 0 )
        return QModelIndex();

    SJobInfo *childItem = parentItem->m_children.at( _row );

    Q_ASSERT( childItem );

    return createIndex( _row, _column, childItem );
}
//=============================================================================
QModelIndex CJobInfoItemModel::parent( const QModelIndex & _index ) const
{
    if ( !_index.isValid() )
        return QModelIndex();

    SJobInfo *childItem = reinterpret_cast<SJobInfo *>( _index.internalPointer() );
    Q_ASSERT( childItem );

    SJobInfo *parentItem = childItem->m_parent;
    Q_ASSERT( parentItem );
// TODO: after deletion of a node I corrupt childItem and get SEGFAULT
    // TODO: REMOVE DEBUG!
    if ( !parentItem )
        cout << "WARNING: parentItem is NULL. For row " << _index.row() << "; column " << _index.column() << endl;

    if ( parentItem == m_rootItem )
        return QModelIndex();

    const int parent_row = parentItem->m_parent->m_children.indexOf( parentItem );
    return createIndex( parent_row, 0, parentItem );
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
void CJobInfoItemModel::jobChanged( SJobInfo * _info )
{
    cout << "jobChanged" << endl;
    if ( !_info )
        return;


    const size_t row = _info->m_parent->m_children.indexOf( _info );
    if ( row < 0 )
        return;

    QModelIndex startIndex = createIndex( row, 0, _info );
    QModelIndex endIndex = createIndex( row, m_Titles.count(), _info );
    emit dataChanged( startIndex, endIndex );
}
//=============================================================================
QModelIndex CJobInfoItemModel::getQModelIndex( SJobInfo *_info, int column ) const
{
    Q_ASSERT( _info );
    if ( !_info )
        return QModelIndex();

    int pid = _info->m_id;
    if ( pid == 0 )
        return QModelIndex();
    int row = 0;
    if ( _info->m_parent )
    {
        row = _info->m_parent->m_children.indexOf( _info );
        Q_ASSERT( row != -1 );
    }
    return createIndex( row, column, _info );
}
//=============================================================================
void CJobInfoItemModel::beginInsertRow( SJobInfo *_info )
{
    // This model only supports insertion of the entire job with all its children
    if ( !_info || NULL != _info->m_parent )
        return;

    cout << "+++ beginInsertRow: " << _info->m_strID << endl;


    // inserting a parent
    _info->m_parent = m_rootItem;
    const int row( _info->m_parent->m_children.count() );
    beginInsertRows( QModelIndex(), row, row );
    m_rootItem->addChild( _info );
    endInsertRows();

    // inserting children
    beginInsertRows( getQModelIndex( _info->m_parent, 0 ), 0, _info->m_parent->m_children.count() );
    endInsertRows();

    emit doneUpdate();
}
//=============================================================================
void CJobInfoItemModel::beginRemoveRow( SJobInfo *_info )
{
    // This model only supports removing of the entire job with all its children
    if ( !_info || _info->m_parent != m_rootItem )
        return;

    cout << "--- beginRemoveRow: " << _info->m_strID << endl;

    // Removing first the children
    const int start_row( 0 );
    const int end_row( _info->m_children.count() - 1 );

    emit beginRemoveRows( getQModelIndex( _info, 0 ), start_row, end_row );
    endRemoveRows();

    // Removing the parent itself
    const int row = _info->m_parent->m_children.indexOf( _info );
    emit beginRemoveRows( getQModelIndex( _info->m_parent, 0 ), row, row );
    // removing the item from the root item children list
    delete _info;
    _info->m_parent->m_children.removeAt( row );
    endRemoveRows();

    emit doneUpdate();
}
//=============================================================================
void CJobInfoItemModel::numberOfActiveJobsChanged( size_t _count )
{
    emit jobsCountUpdated( _count );
}
//=============================================================================
void CJobInfoItemModel::setUpdateInterval( int _newVal )
{
    m_updateInterval = _newVal;
    _newVal <= 0 ? m_jobinfo.stopUpdate() : m_jobinfo.update( m_updateInterval );
}
//=============================================================================
void CJobInfoItemModel::addJob( lsf_jobid_t _jobID )
{
    m_jobinfo.updateNumberOfJobs();
}
//=============================================================================
void CJobInfoItemModel::removeJob( lsf_jobid_t _jobID )
{
    m_jobinfo.updateNumberOfJobs();
}
