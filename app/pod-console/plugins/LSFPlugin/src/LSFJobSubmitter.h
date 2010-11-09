/************************************************************************/
/**
 * @file LSFJobSubmitter.h
 * @brief A thread-class which submits Grid jobs and reports a progress status
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-01-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef LSFJOBSUBMITTER_H_
#define LSFJOBSUBMITTER_H_
//=============================================================================
// gLite plug-in
#include "LsfMng.h"
// STD
#include <sstream>
// Qt
#include <QThread>
#include <QMutex>
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/set.hpp>
// MiscCommon
#include "SysHelper.h"
//=============================================================================
class CLSFJobSubmitter: public QThread
{
        Q_OBJECT

        friend class boost::serialization::access;

    public:
        typedef std::set<lsf_jobid_t> jobslist_t;

    public:
        CLSFJobSubmitter( QObject *parent ): QThread( parent )
        {
            qRegisterMetaType<lsf_jobid_t>( "lsf_jobid_t" );
            init();
        }
        ~CLSFJobSubmitter()
        {
            if ( isRunning() )
                terminate();
        }

    public:
        bool init()
        {
            try
            {
                m_lsf.init();
            }
            catch ( const std::exception &e )
            {
                return false;
            }
            return true;
        }
        const jobslist_t &getParentJobsList() const
        {
            return m_parentJobs;
        }
        void setAllDefault()
        {
            m_parentJobs.clear();
        }
        void setJobScriptFilename( const std::string &_JobScriptFilename )
        {
            m_JobScriptFilename = _JobScriptFilename;
        }
        void setNumberOfWorkers( int _WrkN )
        {
            // setting job array
            std::ostringstream jobname;
            jobname << getpid() << "[1-" << _WrkN << "]";

            m_lsf.addProperty( CLsfMng::JP_SUB_JOB_NAME, jobname.str() );
        }
        void setQueue( const std::string &_queue )
        {
            m_lsf.addProperty( CLsfMng::JP_SUB_QUEUE, _queue );
        }
        void setOutputFiles( const std::string &_path )
        {
            std::string dir( _path );
            MiscCommon::smart_path( &dir );
            MiscCommon::smart_append( &dir, '/' );
            m_lsf.addProperty( CLsfMng::JP_SUB_OUT_FILE, dir + "%J/std_%I.out" );
            m_lsf.addProperty( CLsfMng::JP_SUB_ERR_FILE, dir + "%J/std_%I.err" );
        }
        void removeJob( lsf_jobid_t _jobID, bool _emitSignal = true )
        {
            m_mutex.lock();
            m_parentJobs.erase( _jobID );
            m_mutex.unlock();

            if ( _emitSignal )
                emit removedJob( _jobID );
        }
        void killJob( lsf_jobid_t _jobID )
        {
            m_lsf.killJob( _jobID );
        }
        const CLsfMng &getLSF() const
        {
            return m_lsf;
        }

    signals:
        void changeProgress( int _Val );
        void newJob( lsf_jobid_t _jobID );
        void removedJob( lsf_jobid_t _jobID );
        void sendThreadMsg( const QString &_Msg );

    protected:
        void run()
        {
            emit changeProgress( 0 );

            // Submit a Grid Job
            try
            {
                emit changeProgress( 30 );

                lsf_jobid_t nLastJobID = m_lsf.jobSubmit( m_JobScriptFilename );

                emit changeProgress( 90 );

                m_mutex.lock();
                m_parentJobs.insert( nLastJobID );
                m_mutex.unlock();

                emit newJob( nLastJobID );
            }
            catch ( const std::exception &_e )
            {
                emit sendThreadMsg( tr( _e.what() ) );
                emit changeProgress( 0 );
                return;
            }

            emit changeProgress( 100 );
        }

        // serialization
        template<class Archive>
        void save( Archive & _ar, const unsigned int /*_version*/ ) const
        {
            _ar & BOOST_SERIALIZATION_NVP( m_parentJobs );
        }
        template<class Archive>
        void load( Archive & _ar, const unsigned int _version )
        {
            m_mutex.lock();
            _ar & BOOST_SERIALIZATION_NVP( m_parentJobs );
            m_mutex.unlock();
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        jobslist_t m_parentJobs;
        QMutex m_mutex;
        std::string m_JobScriptFilename;
        CLsfMng m_lsf;
};

BOOST_CLASS_VERSION( CLSFJobSubmitter, 1 )

#endif /*LSFJOBSUBMITTER_H_*/
