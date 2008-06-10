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

        Copyright (c) 2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef JOBSUBMITTER_H_
#define JOBSUBMITTER_H_

// Qt
#include <QtGui>
// BOOST
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
// MiscCommon
#include "gLiteHelper.h"
// GAW
#include "glite-api-wrapper/gLiteAPIWrapper.h"

class CJobSubmitter: public QThread
{
        Q_OBJECT

        typedef glite_api_wrapper::CGLiteAPIWrapper GAW;

        friend class boost::serialization::access;

    public:
        CJobSubmitter( QObject *parent ):
                QThread(parent)
        {
            GAW::Instance().Init();
        }
        ~CJobSubmitter()
        {
            if ( isRunning() )
                terminate();
        }

    public:
        const std::string &getLastJobID()
        {
            // Retrieving a number of children of the parametric job
            emit changeNumberOfJobs( getNumberOfJobs(m_LastJobID) );

            return m_LastJobID;
        }
        void setJDLFileName( const std::string &_JDLfilename )
        {
            m_JDLfilename = _JDLfilename;
        }
        void setEndpoint( const std::string &_Endpoint )
        {
            m_WMPEndpoint = _Endpoint;
        }
        void DelegationCredential() throw (std::exception)
        {
            GAW::Instance().GetJobManager().DelegationCredential( &m_WMPEndpoint );
        }

    signals:
        void changeProgress( int _Val);
        void changeNumberOfJobs( int _Val );
        void sendThreadMsg( const QString &_Msg );

    protected:
        void run()
        {
            emit changeProgress( 0 );
            m_mutex.lock();
            m_LastJobID.clear();
            m_mutex.unlock();

            // Submit a Grid Job
            try
            {
                DelegationCredential();
                emit changeProgress( 30 );

                std::string sLastJobID;
                GAW::Instance().GetJobManager().JobSubmit( m_JDLfilename.c_str(), &sLastJobID );

                // Retrieving a number of children of the parametric job
                emit changeNumberOfJobs( getNumberOfJobs(sLastJobID) );
                m_mutex.lock();
                m_LastJobID = sLastJobID;
                m_mutex.unlock();
            }
            catch ( const std::exception &_e )
            {
                emit sendThreadMsg( tr(_e.what()) );
                emit changeProgress( 0 );
                return;
            }

            emit changeProgress( 100 );
        }

        int getNumberOfJobs( const std::string &_JobID) const
        {
            try
            {
                // Retrieving a number of children of the parametric job
                MiscCommon::StringVector_t jobs;
                MiscCommon::gLite::CJobStatusObj(_JobID).GetChildren( &jobs );
                return (jobs.size());
            }
            catch (...)
            {
            }
            return 0;
        }
        template<class Archive>
        void save(Archive & _ar, const unsigned int /*_version*/) const
        {
            _ar & BOOST_SERIALIZATION_NVP(m_LastJobID);
        }
        template<class Archive>
        void load(Archive & _ar, const unsigned int /*_version*/)
        {
            m_mutex.lock();
            _ar & BOOST_SERIALIZATION_NVP(m_LastJobID);
            m_mutex.unlock();

            DelegationCredential();
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

    private:
        std::string m_LastJobID;
        QMutex m_mutex;
        std::string m_JDLfilename;
        std::string m_WMPEndpoint;
};

BOOST_CLASS_VERSION(CJobSubmitter, 1)

#endif /*JOBSUBMITTER_H_*/
