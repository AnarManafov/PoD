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
#ifndef WORKERSDLG_H_
#define WORKERSDLG_H_

// Qt
#include <QTimer>

// Qt autogen. file
#include "ui_wgWorkers.h"

// MiscCommon
#include "MiscUtils.h"
#include "def.h"

template <class _T>
struct SFindComment
{
    SFindComment( const _T &_CmntSign ): m_CmntSign(_CmntSign)
    {}
    bool operator() ( const _T &_Val ) const
    {
        return ( _Val.find(m_CmntSign) != _Val.npos );
    }
private:
    _T m_CmntSign;
};

class CWorkersDlg: public QWidget
{
        Q_OBJECT
        
    public:
        CWorkersDlg( QWidget *parent = 0 );
        virtual ~CWorkersDlg();

    public slots:
        // Timer
        void update();
        // Monitor List of Workers
        void on_chkShowWorkers_stateChanged( int _Stat );

        void setNumberOfJobs( int _Val, const std::string &_ParentJobID )
        {
            m_LastParentJobID = _ParentJobID;
            setActiveWorkers( 0, _Val );
        }

        // Setting a number of connected workers
        void setActiveWorkers( size_t _Val1, size_t _Val2 = 0 )
        {
            static size_t nTotal = 0;
            if ( _Val2 )
                nTotal = _Val2;
            MiscCommon::tstring strMsg( _T("Monitor connections (available %1 out of %2 worker(s)):") );
            MiscCommon::tstringstream ss;
            ss << _Val1;
            MiscCommon::replace<MiscCommon::tstring>( &strMsg, _T("%1"), ss.str() );
            ss.str("");
            ss << nTotal;
            MiscCommon::replace<MiscCommon::tstring>( &strMsg, _T("%2"), ss.str() );
            m_ui.chkShowWorkers->setText( strMsg.c_str() );
        }

    private:
        void getPROOFCfg( std::string *_FileName );

    private:
        Ui::wgWorkers m_ui;

        QTimer *m_Timer;
        std::string m_CfgFileName;
        std::string m_LastParentJobID;
};

#endif /*WORKERSDLG_H_*/
