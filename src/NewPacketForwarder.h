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
// MiscCommon
#include "INet.h"

namespace PROOFAgent
{

typedef MiscCommon::INet::Socket_t sock_type;
//=============================================================================
    struct SNode
    {
        typedef std::pair<sock_type*, sock_type*> container_type;

        SNode( const sock_type &_first,
               const sock_type &_second ):
                m_sockets( new typename container_type::value_type( _first ),
                           new typename container_type::value_type( _second ) )
        {
        }
        ~SNode()
        {
            delete m_sockets.first;
            delete m_sockets.second;
        }
        bool IsValid()
        {
            return ( m_sockets.first.is_valid() && m_sockets.second.is_valid() );
        }

        container_type m_sockets;
        std::string m_PROOFCfgEntry;
    };

//=============================================================================
    class CNewPacketForwarder
    {
    	typedef boost::shared_ptr<SNode> node_type;
    	typedef std::map<sock_type, node_type> container_type;

        public:
            CNewPacketForwarder();
            virtual ~CNewPacketForwarder();

            void addNode( const sock_type &_first, const sock_type &_second);


        private:
            container_type m_1stSockBasedContainer;
            container_type m_2ndSockBasedContainer;
    };

}

#endif /* NEWPACKETFORWARDER_H_ */
