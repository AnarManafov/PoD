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
// BOOST
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
// MiscCommon
#include "INet.h"

namespace PROOFAgent
{

    typedef MiscCommon::INet::smart_socket sock_type;
//=============================================================================
    struct SNode
    {
        SNode();
        SNode( MiscCommon::INet::Socket_t _first, MiscCommon::INet::Socket_t _second, const std::string &_proofCFGString ):
                m_first( new sock_type( _first ) ),
                m_second( new sock_type( _second ) ),
                m_proofCfgEntry( _proofCFGString ),
                m_active( false )

        {
        }
        ~SNode()
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
        MiscCommon::INet::Socket_t first()
        {
            return m_first->get();
        }
        MiscCommon::INet::Socket_t second()
        {
            return m_second->get();
        }

private:
        sock_type *m_first;
        sock_type *m_second;
        std::string m_proofCfgEntry;
        bool m_active;
        bool m_locked;
    };

//=============================================================================
    class CNodeContainer
    {
        public:
            typedef boost::shared_ptr<SNode> node_type;
            typedef std::map<MiscCommon::INet::Socket_t, node_type> container_type;

        public:
            CNodeContainer();
            virtual ~CNodeContainer();

            void addNode( node_type _node );
            void removeNode1stBase( MiscCommon::INet::Socket_t _fd );
            void removeNode2ndBase( MiscCommon::INet::Socket_t _fd );
            SNode *getNode1stBase( MiscCommon::INet::Socket_t _fd );
            SNode *getNode2ndBase( MiscCommon::INet::Socket_t _fd );

        private:
            container_type m_1stSockBasedContainer;
            container_type m_2ndSockBasedContainer;
    };

//=============================================================================
    class CNewPacketForwarder
    {
            typedef std::queue<MiscCommon::INet::Socket_t> taskqueue_t;
            enum {ReadFromFirst, ReadFromSecond};
        public:
            void start( size_t _threadsCount );
            void pushTask( SNode );
            void removeTopTask();

        private:
            boost::thread_group m_threads;
            taskqueue_t m_tasks;
    };

}

#endif /* NEWPACKETFORWARDER_H_ */
