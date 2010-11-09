/************************************************************************/
/**
 * @file
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2010-03-30
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 Anar Manafov. All rights reserved.
*************************************************************************/
// STD
#include <iostream>
//
#include "JobInfoItemModel.h"
//=============================================================================
using namespace std;
using namespace pbs_plug;

const short g_columnCount = 2;
//=============================================================================
CJobInfoItemModel::CJobInfoItemModel( CPbsJobSubmitter *_submitter,
                                      int _updateInterval,
                                      QObject * _parent ):
        QAbstractItemModel( _parent ),
        m_jobinfo( _submitter ),
        m_updateInterval( _updateInterval ),
        m_rootItem( new SJobInfo( "" ) )
{
    _setupHeader();
    _setupJobsContainer();
}
//=============================================================================
CJobInfoItemModel::~CJobInfoItemModel()
{
//   delete m_rootItem;
}
//=============================================================================
void CJobInfoItemModel::_setupJobsContainer()
{
    connect( &m_jobinfo, SIGNAL( jobChanged( SJobInfo* ) ), this, SLOT( jobChanged( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( addJob( SJobInfo* ) ), this, SLOT( insertJobs( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( removeJob( SJobInfo* ) ), this, SLOT( removeJobs( SJobInfo* ) ) );
    connect( &m_jobinfo, SIGNAL( numberOfActiveJobsChanged( size_t ) ), this, SLOT( numberOfActiveJobsChanged( size_t ) ) );

    m_jobinfo.update( m_updateInterval );
}
//=============================================================================
int CJobInfoItemModel::rowCount( const QModelIndex &_parent ) const
{
    if ( _parent.column() > 0 )
        return 0;

    SJobInfo *parentItem = NULL;

    if ( !_parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = reinterpret_cast<SJobInfo *>( _parent.internalPointer() );

    return parentItem->m_children.count();
}
//=============================================================================
int CJobInfoItemModel::columnCount( const QModelIndex & _parent ) const
{
    if ( _parent.column() > 0 )
        return 0;
    return g_columnCount;
}
//=============================================================================
QVariant CJobInfoItemModel::data( const QModelIndex &_index, int _role ) const
{
    // This function is called a lot of times and it therefore needs to be very optimized

    if ( !_index.isValid() )
        return QVariant();
    if ( _index.column() >= g_columnCount )
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
    if ( _section < 0 || _section >= g_columnCount )
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
QModelIndex CJobInfoItemModel::getQModelIndex( SJobInfo *_info, int column ) const
{
    Q_CHECK_PTR( _info );

    if ( _info->m_id.empty() ) // this is our fake item - which is a super parent for all jobs
        return QModelIndex();

    const int row( _info->parent()->m_children.indexOf( _info ) );
    Q_ASSERT( row != -1 );
    if ( -1 == row )
    {
        cout << "WARNING: can't find item: ";
        return QModelIndex();
    }

    return createIndex( row, column, _info );
}
//=============================================================================
QModelIndex CJobInfoItemModel::index( int _row, int _column, const QModelIndex & _parent ) const
{
    if ( _column < 0 || _column >= g_columnCount || _row < 0 || _parent.column() > 0 )
        return QModelIndex();

    SJobInfo *parentItem = NULL;
    // it is a root item, the one who dosn't have parent
    if ( !_parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = reinterpret_cast<SJobInfo *>( _parent.internalPointer() );

    Q_CHECK_PTR( parentItem );

    if ( !parentItem || _row >= parentItem->m_children.count() )
        return QModelIndex();

    return createIndex( _row, _column, parentItem->m_children[_row] );
}
//=============================================================================
QModelIndex CJobInfoItemModel::parent( const QModelIndex & _index ) const
{
    if ( !_index.isValid() )
        return QModelIndex();

    SJobInfo *childItem = reinterpret_cast<SJobInfo *>( _index.internalPointer() );
    Q_CHECK_PTR( childItem );
    if ( !childItem )
        return QModelIndex();

    return getQModelIndex( childItem->parent(), 0 );
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
    if ( !_info )
        return;

    emit dataChanged( getQModelIndex( _info, 0 ),
                      getQModelIndex( _info, g_columnCount - 1 ) );
}
//=============================================================================
void CJobInfoItemModel::insertJobs( SJobInfo *_info )
{
    // This model only supports insertion of the entire job with all its children
    if ( !_info || NULL != _info->parent() )
        return;

    // inserting a parent
    const int row( m_rootItem->m_children.count() );
    beginInsertRows( getQModelIndex( m_rootItem, 0 ), row, row );
    _info->setParent( m_rootItem );
    m_rootItem->addChild( _info );
    endInsertRows();

    // inserting children
    const int start_row( 0 );
    const int end_row( _info->m_children.count() - 1 );
    beginInsertRows( getQModelIndex( _info, 0 ), start_row, end_row );
    endInsertRows();

    emit doneUpdate();
}
//=============================================================================
void CJobInfoItemModel::removeJobs( SJobInfo *_info )
{
    // This model only supports removing of the entire job with all its children
    if ( !_info || _info->parent() != m_rootItem )
        return;

    // Removing first the children
    const int start_row( 0 );
    const int end_row( _info->m_children.count() - 1 );

    emit beginRemoveRows( getQModelIndex( _info, 0 ), start_row, end_row );
    _info->removeAllChildren();
    endRemoveRows();

    // Removing the parent itself
    const int row = _info->parent()->m_children.indexOf( _info );
    if ( row < 0 )
        cerr << "Error: Can't remove " << _info->m_strID << endl;

    emit beginRemoveRows( getQModelIndex( _info->parent(), 0 ), row, row );
    // removing the item from the root item children list
    _info->parent()->m_children.removeAt( row );
    delete _info;
    _info = NULL;
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
void CJobInfoItemModel::addJob( const CPbsMng::jobID_t &_jobID )
{
    m_jobinfo.updateNumberOfJobs();
}
//=============================================================================
void CJobInfoItemModel::removeJob( const CPbsMng::jobID_t &_jobID )
{
    m_jobinfo.updateNumberOfJobs();
}
//=============================================================================
void CJobInfoItemModel::removeAllCompletedJobs()
{
    m_jobinfo.removeAllCompletedJobs();
}
