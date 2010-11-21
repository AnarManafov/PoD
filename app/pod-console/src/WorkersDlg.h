/************************************************************************/
/**
 * @file WorkersDlg.h
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 *//*

        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           2007-08-24
        last changed by:   $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef WORKERSDLG_H_
#define WORKERSDLG_H_
//=============================================================================
// Qt autogen. file
#include "ui_wgWorkers.h"
// MiscCommon
#include "MiscUtils.h"
//=============================================================================
class QFileSystemWatcher;
//=============================================================================
template <class _T>
struct SFindComment
{
        SFindComment( const typename _T::value_type &_CmntSign ): m_CmntSign( _CmntSign )
        {}
        bool operator()( const _T &_Val ) const
        {
            return ( _Val.find( m_CmntSign ) != _Val.npos );
        }
    private:
        typename _T::value_type m_CmntSign;
};
//=============================================================================
class CWorkersDlg: public QWidget
{
        Q_OBJECT

    public:
        CWorkersDlg( QWidget *parent = NULL );
        virtual ~CWorkersDlg();

        void init( const std::string &_proofCFG );

    public slots:
        void update();

        void setNumberOfJobs( int _Val )
        {
            setActiveWorkers( getWorkersFromPROOFCfg(), _Val, false );
        }

        // Setting a number of connected workers
        void setActiveWorkers( size_t _Val1, size_t _Val2 = 0, bool _onlyTheFirst = true );
        bool isWatching();
        void restartWatcher();

    protected:
        void showEvent( QShowEvent* );

    private:
        int getWorkersFromPROOFCfg();

    private:
        Ui::wgWorkers m_ui;
        std::string m_CfgFileName;
        QFileSystemWatcher *m_watcher;
};

#endif /*WORKERSDLG_H_*/
