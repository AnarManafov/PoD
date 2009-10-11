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
            typedef enum ENodeSocket {nodeSocketFirst, nodeSocketSecond} ENodeSocket_t;
            REGISTER_LOG_MODULE( "Node" )
            CNode();
            CNode( MiscCommon::INet::Socket_t _first, MiscCommon::INet::Socket_t _second,
                   const std::string &_proofCFGString, unsigned int _readBufSize ):
                    m_proofCfgEntry( _proofCFGString ),
                    m_active( false ),
                    m_inUseFirst( false ),
                    m_inUseSecond( true ),
                    m_bufFirst( _readBufSize ),
                    m_bufSecond( _readBufSize )
                    //  m_bytesToSend( 0 )

            {
                m_first = new sock_type( _first );
                m_first->set_nonblock();
                m_second = new sock_type( _second );
                m_second->set_nonblock();
            }
            ~CNode()
            {
                delete m_first;
                delete m_second;
            }
            void update( MiscCommon::INet::Socket_t _fd, ENodeSocket_t _which )
            {
                sock_type *sock( nodeSocketFirst == _which ? m_first : m_second );

                sock_type tmp( sock->get() );
                *sock = _fd;
                sock->set_nonblock();
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
            void setInUse( bool _Val, ENodeSocket_t _which )
            {
                switch ( _which )
                {
                    case nodeSocketFirst:
                        m_inUseFirst = _Val;
                        break;
                    case nodeSocketSecond:
                        m_inUseSecond = _Val;
                        break;
                }
            }
            bool isInUse( ENodeSocket_t _which )
            {
                return ( nodeSocketFirst == _which ? m_inUseFirst : m_inUseSecond );
            }
            MiscCommon::INet::Socket_t getSocket( ENodeSocket_t _which )
            {
                return ( nodeSocketFirst == _which ? m_first->get() : m_second->get() );
            }
            /// returns 0 if everything is OK, -1 if socket or sockets are not valid
            int dealWithData( ENodeSocket_t _which );
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
            bool m_inUseFirst;
            bool m_inUseSecond;
            MiscCommon::BYTEVector_t m_bufFirst;
            MiscCommon::BYTEVector_t m_bufSecond;
            //size_t m_bytesToSend;
    };
}

#endif /* NODE_H_ */
