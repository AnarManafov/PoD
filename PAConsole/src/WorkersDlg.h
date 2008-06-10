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
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// MiscCommon
#include "MiscUtils.h"
#include "def.h"

template <class _T>
struct SFindComment
{
    SFindComment( const typename _T::value_type &_CmntSign ): m_CmntSign(_CmntSign)
    {}
    bool operator() ( const _T &_Val ) const
    {
        return ( _Val.find(m_CmntSign) != _Val.npos );
    }
private:
    typename _T::value_type m_CmntSign;
};

class CWorkersDlg: public QWidget
{
        Q_OBJECT

        friend class boost::serialization::access;

    public:
        CWorkersDlg( QWidget *parent = NULL );
        virtual ~CWorkersDlg();

    public slots:
        // Timer
        void update();
        // Monitor List of Workers
        void on_chkShowWorkers_stateChanged( int _Stat );

        void setNumberOfJobs( int _Val )
        {
            setActiveWorkers( getWorkersFromPROOFCfg(), _Val );
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
        int getWorkersFromPROOFCfg();

        template<class Archive>
        void save(Archive & _ar, const unsigned int /*_version*/) const
        {
            _ar & BOOST_SERIALIZATION_NVP(m_bMonitorWorkers);
        }
        template<class Archive>
        void load(Archive & _ar, const unsigned int /*_version*/)
        {
            _ar & BOOST_SERIALIZATION_NVP(m_bMonitorWorkers);

            on_chkShowWorkers_stateChanged( m_bMonitorWorkers ? Qt::Checked : Qt::Unchecked );
            m_ui.chkShowWorkers->setCheckState( m_bMonitorWorkers ? Qt::Checked : Qt::Unchecked );
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        Ui::wgWorkers m_ui;
        QTimer *m_Timer;
        std::string m_CfgFileName;
        bool m_bMonitorWorkers;
};

BOOST_CLASS_VERSION(CWorkersDlg, 1)

#endif /*WORKERSDLG_H_*/
