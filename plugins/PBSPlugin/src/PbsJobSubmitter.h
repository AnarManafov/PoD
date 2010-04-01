/************************************************************************/
/**
 * @file
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2010-03-30
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PBSJOBSUBMITTER_H_
#define PBSJOBSUBMITTER_H_
//=============================================================================
// gLite plug-in
#include "PbsMng.h"
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
namespace pbs_plug
{
//=============================================================================
    class CPbsJobSubmitter: public QThread
    {
            Q_OBJECT

            friend class boost::serialization::access;

        public:
            // <parent job id, num of children>
            typedef std::map<CPbsMng::jobID_t, size_t> jobslist_t;

        public:
            CPbsJobSubmitter( QObject *parent ): QThread( parent )
            {
            }
            ~CPbsJobSubmitter()
            {
                if ( isRunning() )
                    terminate();
            }

        public:
//        const jobslist_t &getParentJobsList() const
//        {
//            return m_parentJobs;
//        }
//        void setAllDefault()
//        {
//            m_parentJobs.clear();
//        }
            void setJobScriptFilename( const std::string &_JobScriptFilename )
            {
                m_JobScriptFilename = _JobScriptFilename;
            }
            void setNumberOfWorkers( size_t _WrkN )
            {
                m_numberOfWrk = _WrkN;
            }
            void setQueue( const std::string &_queue )
            {
                m_queue = _queue;
            }
//        void setOutputFiles( const std::string &_path )
//        {
//            std::string dir( _path );
//            MiscCommon::smart_path( &dir );
//            MiscCommon::smart_append( &dir, '/' );
//            m_lsf.addProperty( CLsfMng::JP_SUB_OUT_FILE, dir + "%J/std_%I.out" );
//            m_lsf.addProperty( CLsfMng::JP_SUB_ERR_FILE, dir + "%J/std_%I.err" );
//        }
//        void removeJob( lsf_jobid_t _jobID, bool _emitSignal = true )
//        {
//            m_mutex.lock();
//            m_parentJobs.erase( _jobID );
//            m_mutex.unlock();
//
//            if ( _emitSignal )
//                emit removedJob( _jobID );
//        }
//        void killJob( lsf_jobid_t _jobID )
//        {
//            m_lsf.killJob( _jobID );
//        }
            const CPbsMng &getPBS() const
            {
                return m_pbs;
            }
//
//    signals:
//        void changeProgress( int _Val );
//        void newJob( lsf_jobid_t _jobID );
//        void removedJob( lsf_jobid_t _jobID );
//        void sendThreadMsg( const QString &_Msg );
//
    protected:
        void run()
        {
//            emit changeProgress( 0 );
//
//            // Submit a Grid Job
//            try
//            {
//                emit changeProgress( 30 );
//
//                lsf_jobid_t nLastJobID = m_lsf.jobSubmit( m_JobScriptFilename );
//
//                emit changeProgress( 90 );
//
//                m_mutex.lock();
//                m_parentJobs.insert( nLastJobID );
//                m_mutex.unlock();
//
//                emit newJob( nLastJobID );
//            }
//            catch ( const std::exception &_e )
//            {
//                emit sendThreadMsg( tr( _e.what() ) );
//                emit changeProgress( 0 );
//                return;
//            }
//
//            emit changeProgress( 100 );
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
            size_t m_numberOfWrk;
            std::string m_queue;
            CPbsMng m_pbs;
    };

}

BOOST_CLASS_VERSION( pbs_plug::CPbsJobSubmitter, 1 )

#endif /*PBSJOBSUBMITTER_H_*/
