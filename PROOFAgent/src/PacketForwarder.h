/************************************************************************/
/**
 * @file $$File name$$
 * @brief $$File comment$$
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*
 
        version number:    $LastChangedRevision$
        created by:        Anar Manafov
                           $$date$$
        last changed by:   $LastChangedBy$ $LastChangedDate$
 
        Copyright (c) 2006,2007 GSI GridTeam. All rights reserved.
*************************************************************************/
#ifndef PROOFAGENTPACKETFORWARDER_H
#define PROOFAGENTPACKETFORWARDER_H

// Our
#include "INet.h"
#include "LogImp.h"

namespace PROOFAgent
{

    class CPacketForwarder: MiscCommon::CLogImp<CPacketForwarder>
    {
        public:
            CPacketForwarder( MiscCommon::INet::Socket_t &_Socket, unsigned short _nPort ) :
                    m_Socket( _Socket ),
                    m_nPort( _nPort )
            {}
            ~CPacketForwarder()
            {}
            REGISTER_LOG_MODULE( PacketForwarder )

        public:
            MiscCommon::ERRORCODE Start( );

        private:
            MiscCommon::INet::smart_socket m_Socket;
            unsigned short m_nPort;
    };

}

#endif
