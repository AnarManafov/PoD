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

using namespace std;

CJobInfoItemModel::CJobInfoItemModel( const CLSFJobSubmitter *_lsfsubmitter, QObject * _parent ):
        QAbstractItemModel( _parent ),
        m_jobinfo(_lsfsubmitter)
{
    _setupHeader();
    _setupJobsContainer();
}

CJobInfoItemModel::~CJobInfoItemModel()
{
}

int CJobInfoItemModel::rowCount( const QModelIndex &parent ) const
{
    return m_jobinfo.getCount();
}

int CJobInfoItemModel::columnCount( const QModelIndex & parent ) const
{
    return m_Titles.count();
}

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

QModelIndex CJobInfoItemModel::index( int _row, int _column, const QModelIndex & _parent ) const
{
    if ( _row < 0 )
        return QModelIndex();
    if ( _column < 0 || _column >= m_Titles.count() )
        return QModelIndex();

    if ( static_cast<int>( m_jobinfo.getCount() ) <= _row )
        return QModelIndex();

    SJobInfo *parent_job = NULL;

    if ( _parent.isValid() )
        parent_job = reinterpret_cast<SJobInfo *>( _parent.internalPointer() );
    else
        return createIndex( _row, _column, m_jobinfo.at( _row ) );

    if ( static_cast<int>(parent_job->m_children.size()) > _row )
        return createIndex(_row, _column, parent_job->m_children[_row].get());
    else
    {
        return QModelIndex();
    }

    return createIndex( _row, _column, m_jobinfo.at( _row ) );
}

Qt::ItemFlags CJobInfoItemModel::flags( const QModelIndex & _index ) const
{
    if ( !_index.isValid() )
        return 0;

    return QAbstractItemModel::flags( _index ) & ( ~Qt::ItemIsEditable );
}

QModelIndex CJobInfoItemModel::parent( const QModelIndex & _index ) const
{
    return QModelIndex();
}

SJobInfo *CJobInfoItemModel::getJobInfoAtIndex(int _index) const
{
    if ( _index < 0 )
        return NULL;
    if ( static_cast<int>( m_jobinfo.getCount() ) <= _index )
        return NULL;

    return m_jobinfo.at( _index );
}

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

void CJobInfoItemModel::jobChanged( SJobInfo *_info )
{
    Q_ASSERT( _info );

    const size_t row = m_jobinfo.getIndex( _info );
    if ( row >= m_jobinfo.getCount() )
        return;

    QModelIndex startIndex = createIndex( row, 0, _info );
    QModelIndex endIndex = createIndex( row, m_Titles.count() - 1, _info );
    emit dataChanged( startIndex, endIndex );
}

void CJobInfoItemModel::beginInsertRow( SJobInfo *_info )
{
    Q_ASSERT( _info );
    const int row = m_jobinfo.getCount();
    beginInsertRows( QModelIndex(), row, row );

}

void CJobInfoItemModel::endInsertRow()
{
    endInsertRows();
}

void CJobInfoItemModel::beginRemoveRow( SJobInfo *_info )
{
    Q_ASSERT( _info );

    const size_t row = m_jobinfo.getIndex( _info );
    if ( row >= m_jobinfo.getCount() )
        return;

    return beginRemoveRows( QModelIndex(), row, row );
}

void CJobInfoItemModel::endRemoveRow()
{
    endRemoveRows();
}

void CJobInfoItemModel::_setupJobsContainer()
{
    connect( &m_jobinfo, SIGNAL( jobChanged( SJobInfo * ) ), this, SLOT( jobChanged( SJobInfo * ) ) );
    connect( &m_jobinfo, SIGNAL( beginAddJob( SJobInfo * ) ), this, SLOT( beginInsertRow( SJobInfo * ) ) );
    connect( &m_jobinfo, SIGNAL( endAddJob() ), this, SLOT( endInsertRow() ) );
    connect( &m_jobinfo, SIGNAL( beginRemoveJob( SJobInfo * ) ), this, SLOT( beginRemoveRow( SJobInfo * ) ) );
    connect( &m_jobinfo, SIGNAL( endRemoveJob() ), this, SLOT( endRemoveRow() ) );

    m_jobinfo.update( 10000 );
}
