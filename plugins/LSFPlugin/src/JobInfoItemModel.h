/************************************************************************/
/**
 * @file JobInfoItemModel.h
 * @brief $$File comment$$
 * @author Anar Manafov Anar.Manafov@gmail.com
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2009-03-08
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 Anar Manafov. All rights reserved.
*************************************************************************/
#ifndef JOBINFOITEMMODEL_H_
#define JOBINFOITEMMODEL_H_
//=============================================================================
// LSF plug-in
#include "JobsContainer.h"
// Qt
#include <QAbstractItemModel>
#include <QStringList>
//=============================================================================
class CJobInfoItemModel: public QAbstractItemModel
{
        Q_OBJECT

    public:
        CJobInfoItemModel( const CLSFJobSubmitter *_lsfsubmitter, int _updateInterval, QObject * _parent = NULL );
        virtual ~CJobInfoItemModel();

    public:
        /** The headings in the model.
         *  The order here is the order that they are shown in.
         *  If you change this, make sure you also change the
         *  _setupHeader method
         */
        enum { TitleJobID, TitleJobStatus };

        int rowCount( const QModelIndex &parent = QModelIndex() ) const;
        int columnCount( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data( const QModelIndex &_index, int _role ) const;
        QVariant headerData( int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole ) const;
        QModelIndex index( int _row, int _column, const QModelIndex & _parent = QModelIndex() ) const;
        virtual Qt::ItemFlags flags( const QModelIndex & _index ) const;
        QModelIndex parent( const QModelIndex &_index ) const;
        void setUpdateInterval( int _newVal );

    signals:
        void doneUpdate();
        void jobsCountUpdated( size_t _count );

    private slots:
        void jobChanged( SJobInfo *_info );
        void insertJobs( SJobInfo *_info );
        void removeJobs( SJobInfo *_info );
        void addJob( lsf_jobid_t _jobID );
        void removeJob( lsf_jobid_t _jobID );
        void numberOfActiveJobsChanged( size_t _count );


    private:
        void _setupJobsContainer();
        void _setupHeader();
        QModelIndex getQModelIndex( SJobInfo *_info, int column ) const;

    private:
        CJobsContainer m_jobinfo;
        /**
         * A translated list of column titles in the order we want to display them. Used in headerData().
         * */
        QStringList m_Titles;
        int m_updateInterval;
        SJobInfo *m_rootItem;
};

#endif /* JOBINFOITEMMODEL_H_ */
