/************************************************************************/
/**
 * @file JobSubmitter.h
 * @brief A thread-class which submits Grid jobs and reports a progress status
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-06-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2008 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef JOBSUBMITTER_H_
#define JOBSUBMITTER_H_
//=============================================================================
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
#include "gLiteHelper.h"
// GAW
#include "gLiteAPIWrapper.h"
//=============================================================================
class CJobSubmitter: public QThread
{
        Q_OBJECT

        typedef glite_api_wrapper::CGLiteAPIWrapper GAW;

        friend class boost::serialization::access;

    public:
        typedef std::set<std::string> jobslist_t;

    public:
        CJobSubmitter( QObject *parent ): QThread( parent )
        {
            GAW::Instance().Init();
        }
        ~CJobSubmitter()
        {
            if( isRunning() )
                terminate();
        }

    public:
        const jobslist_t &getActiveJobList()
        {
            // Retrieving a number of children of the parametric job
            emit changeNumberOfJobs( getNumberOfJobs() );

            return m_JobsList;
        }
        void setAllDefault()
        {
            emit changeNumberOfJobs( 0 );
            m_JobsList.clear();
        }
        void setJDLFileName( const std::string &_JDLfilename )
        {
            m_JDLfilename = _JDLfilename;
        }
        void setEndpoint( const std::string &_Endpoint )
        {
            m_WMPEndpoint = _Endpoint;
        }
        void DelegationCredential() throw( std::exception )
        {
            GAW::Instance().GetJobManager().DelegationCredential( &m_WMPEndpoint );
        }
        void RemoveJob( const std::string &_JobID )
        {
            m_mutex.lock();
            m_JobsList.erase( _JobID );
            m_mutex.unlock();

            emit changeNumberOfJobs( getNumberOfJobs() );
        }

    signals:
        void changeProgress( int _Val );
        void changeNumberOfJobs( int _Val );
        void sendThreadMsg( const QString &_Msg );

    protected:
        void run()
        {
            emit changeProgress( 0 );

            // Submit a Grid Job
            try
            {
                DelegationCredential();
                emit changeProgress( 30 );

                std::string sLastJobID;
                GAW::Instance().GetJobManager().JobSubmit( m_JDLfilename.c_str(), &sLastJobID );

                emit changeProgress( 90 );

                m_mutex.lock();
                m_JobsList.insert( sLastJobID );
                m_mutex.unlock();

                // Retrieving a number of children of the parametric job
                emit changeNumberOfJobs( getNumberOfJobs() );
            }
            catch( const std::exception &_e )
            {
                emit sendThreadMsg( tr( _e.what() ) );
                emit changeProgress( 0 );
                return;
            }

            emit changeProgress( 100 );
        }

        // this function is very "expensive",
        // we therefore use it only via signal only when number of jobs may be changed.
        // Users can't call it any time they want.
        int getNumberOfJobs() const
        {
            if( m_JobsList.empty() )
                return 0;

            try
            {
                jobslist_t::const_iterator iter = m_JobsList.begin();
                jobslist_t::const_iterator iter_end = m_JobsList.end();
                // Retrieving a number of children of the parametric job
                size_t num( 0 );
                MiscCommon::StringVector_t jobs;
                for( ; iter != iter_end; ++iter )
                {
                    MiscCommon::gLite::CJobStatusObj( *iter ).GetChildren( &jobs );
                    num += jobs.size();
                    jobs.clear();
                }
                return ( num );
            }
            catch( ... )
                {}
            return 0;
        }

        // serialization
        template<class Archive>
        void save( Archive & _ar, const unsigned int /*_version*/ ) const
        {
            _ar & BOOST_SERIALIZATION_NVP( m_JobsList );
        }
        template<class Archive>
        void load( Archive & _ar, const unsigned int _version )
        {
            m_mutex.lock();
            if( _version >= 2 )  // TODO: make CJobSubmitter v1 in  PAconsole version 1.0.6 depreciated
                _ar & BOOST_SERIALIZATION_NVP( m_JobsList );
            m_mutex.unlock();

            try
            {
                DelegationCredential();
            }
            catch( ... )
                {}
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        jobslist_t m_JobsList;
        QMutex m_mutex;
        std::string m_JDLfilename;
        std::string m_WMPEndpoint;
};

BOOST_CLASS_VERSION( CJobSubmitter, 2 )

#endif /*JOBSUBMITTER_H_*/
