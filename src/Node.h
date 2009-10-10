/************************************************************************/
/**
 * @file Node.h
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef NODE_H_
#define NODE_H_
// BOOST
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
// MiscCommon
#include "INet.h"
#include "LogImp.h"
#include "def.h"

namespace PROOFAgent
{

    typedef MiscCommon::INet::smart_socket sock_type;
//=============================================================================
    class CNode: public MiscCommon::CLogImp<CNode>
    {
        public:
            REGISTER_LOG_MODULE( "Node" )
            CNode();
            CNode( MiscCommon::INet::Socket_t _first, MiscCommon::INet::Socket_t _second,
                   const std::string &_proofCFGString, unsigned int _readBufSize ):
                    m_first( new sock_type( _first ) ),
                    m_second( new sock_type( _second ) ),
                    m_proofCfgEntry( _proofCFGString ),
                    m_active( false ),
                    m_inUse( false ),
                    m_buf( _readBufSize ),
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
            void setInUse( bool _Val )
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
            boost::mutex m_mutex;
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

}

#endif /* NODE_H_ */
