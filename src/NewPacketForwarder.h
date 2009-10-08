/************************************************************************/
/**
 * @file NewPacketForwarder.h
 * @brief Header file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-09-28
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/

#ifndef NEWPACKETFORWARDER_H_
#define NEWPACKETFORWARDER_H_
// STD
#include <queue>
#include <csignal>
// BOOST
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
// MiscCommon
#include "INet.h"
#include "LogImp.h"

namespace PROOFAgent
{
// TODO: Move it to config.
    const unsigned int g_BUF_SIZE = 5000;

    typedef MiscCommon::INet::smart_socket sock_type;
//=============================================================================
    class CNode: public MiscCommon::CLogImp<CNode>
    {
        public:
            REGISTER_LOG_MODULE( "Node" )
            CNode();
            CNode( MiscCommon::INet::Socket_t _first, MiscCommon::INet::Socket_t _second, const std::string &_proofCFGString ):
                    m_first( new sock_type( _first ) ),
                    m_second( new sock_type( _second ) ),
                    m_proofCfgEntry( _proofCFGString ),
                    m_active( false ),
                    m_inUse(false),
                    m_buf( g_BUF_SIZE ),
                    m_bytesToSend( 0 )

            {
            }
            ~CNode()
            {
                delete m_first;
                delete m_second;
            }
            void updateFirst( MiscCommon::INet::Socket_t _fd )
            {
                *m_first = _fd;
            }
            void updateSecond( MiscCommon::INet::Socket_t _fd )
            {
                *m_second = _fd;
            }
            bool activate()
            {
                m_active = isValid();
                return m_active;
            }
            void disable()
            {
                m_active = false;
            }
            bool isActive()
            {
                return m_active;
            }
            bool isValid()
            {
                return ( NULL != m_first && NULL != m_second &&
                         m_first->is_valid() && m_second->is_valid() );
            }
            void setInUse( bool _Val)
            {
            	m_inUse = _Val;
            }
            bool isInUse()
            {
            	return m_inUse;
            }
            MiscCommon::INet::Socket_t first()
            {
                return m_first->get();
            }
            MiscCommon::INet::Socket_t second()
            {
                return m_second->get();
            }
            sock_type *pairedWith( MiscCommon::INet::Socket_t _fd )
            {
                return (( m_first->get() == _fd ) ? m_second : m_first );
            }
            sock_type *socketByFD( MiscCommon::INet::Socket_t _fd )
            {
                return (( m_first->get() == _fd ) ? m_first : m_second );
            }
            void setNonblock()
            {
            	m_first->set_nonblock();
            	m_second->set_nonblock();
            }
            /// returns 0 if everything is OK, -1 if socket or sockets are not valid
            int dealWithData( MiscCommon::INet::Socket_t _fd );
            std::string getPROOFCfgEntry()
            {
                return m_proofCfgEntry;
            }
            void ReportPackage( MiscCommon::INet::Socket_t _socket1, MiscCommon::INet::Socket_t _socket2,
                                const MiscCommon::BYTEVector_t &_buf );
        private:
            sock_type *m_first;
            sock_type *m_second;
            std::string m_proofCfgEntry;
            bool m_active;
            bool m_inUse;
            MiscCommon::BYTEVector_t m_buf;
            size_t m_bytesToSend;
    };

//=============================================================================
    class CNodeContainer
    {
        public:
            typedef boost::shared_ptr<CNode> node_type;
            typedef std::map<MiscCommon::INet::Socket_t, node_type> container_type;
            typedef std::set<node_type> unique_container_type;

        public:
            CNodeContainer();
            virtual ~CNodeContainer();

            void addNode( node_type _node );
            // is not thread safe
            void removeNode( MiscCommon::INet::Socket_t _fd );
            void removeBadNodes();
            node_type getNode( MiscCommon::INet::Socket_t _fd );
            const container_type *const getContainer() const
            {
                return &m_sockBasedContainer;
            }
            const unique_container_type *const getNods()
            {
                return &m_nodes;
            }

        private:
            // Contains all sockets from nodes,
            // that simply means for each node it keeps two sockets
            container_type m_sockBasedContainer;
            // Contains only pointers to nodes
            unique_container_type m_nodes;
    };

//=============================================================================
    class CThreadPool: public MiscCommon::CLogImp<CThreadPool>
    {
            typedef std::pair<MiscCommon::INet::Socket_t, CNode*> task_t;
            typedef std::queue<task_t*> taskqueue_t;
        public:
            REGISTER_LOG_MODULE( "ThreadPool" )

            CThreadPool( size_t _threadsCount, const std::string &_signalPipePath );
            ~CThreadPool();

            void pushTask( MiscCommon::INet::Socket_t _fd, CNode* _node );
            void execute();
            void stop( bool processRemainingJobs = false );

        private:
            boost::thread_group m_threads;
            taskqueue_t m_tasks;
            boost::mutex m_mutex;
            boost::condition m_threadNeeded;
            boost::condition m_threadAvailable;
            bool m_stopped;
            bool m_stopping;
            int m_fdSignalPipe;
    };

}

#endif /* NEWPACKETFORWARDER_H_ */
