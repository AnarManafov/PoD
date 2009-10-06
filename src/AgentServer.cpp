/************************************************************************/
/**
 * @file AgentServer.cpp
 * @brief Packet Forwarder's implementation
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009 GSI GridTeam. All rights reserved.
*************************************************************************/
// BOOST
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "ErrorCode.h"
#include "INet.h"
// PROOFAgent
#include "AgentServer.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
namespace inet = MiscCommon::INet;
//=============================================================================
const size_t g_READ_READY_INTERVAL = 4;
extern sig_atomic_t graceful_quit;

// TODO: Move to config or make it autodetectable...
const size_t g_numThreads = 4;
//=============================================================================
namespace PROOFAgent
{

//=============================================================================
    CAgentServer::CAgentServer( const SOptions_t &_data ):
            CAgentBase( _data.m_podOptions.m_server.m_common )
           // m_threadPool( g_numThreads )
    {
        m_Data = _data.m_podOptions.m_server;
        m_serverInfoFile = _data.m_serverInfoFile;

        //InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;
    }

//=============================================================================
    CAgentServer::~CAgentServer()
    {
        deleteServerInfoFile();
    }

//=============================================================================
    void CAgentServer::ThreadWorker()
    {
        DebugLog( erOK, "Creating a PROOF configuration file..." );
        createPROOFCfg();
        try
        {
            readServerInfoFile( m_serverInfoFile );

            inet::CSocketServer server;
            server.Bind( m_agentServerListenPort );
            server.Listen( 100 ); // TODO: Move this number of queued clients to config
            server.GetSocket().set_nonblock(); // Nonblocking server socket

            // Add main server's socket to the list of sockets to select
            f_serverSocket = server.GetSocket().get();
            m_socksToSelect.insert( f_serverSocket );
            DebugLog( erOK, "Entering into the \"select\" loop..." );
            while ( true )
            {
                // TODO: we need to check real PROOF port here (from cfg)
                if ( !IsPROOFReady( 0 ) )
                {
                    FaultLog( erError, "Can't connect to PROOF/XRD service." );
                    graceful_quit = 1;
                }

                // Checking whether signal has arrived
                if ( graceful_quit )
                {
                    InfoLog( erOK, "STOP signal received." );
                  //  m_threadPool.stop();
                    return ;
                }

                // ------------------------
                // A Global "select"
                // ------------------------Ê
                mainSelect( server );
            }
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
        }
    }

//=============================================================================
    void CAgentServer::mainSelect( const inet::CSocketServer &_server )
    {
        fd_set readset;
        FD_ZERO( &readset );

        // TODO: implement poll or check that a number of sockets is not higher than 1024 (limitations of "select" )
        Sockets_type::const_iterator iter = m_socksToSelect.begin();
        Sockets_type::const_iterator iter_end = m_socksToSelect.end();
        for ( ; iter != iter_end; ++iter )
        {
//            // don't include node which are being processed at this moment
//            CNodeContainer::node_type node = m_nodes.getNode( *iter );
//            if ( node.get() == NULL )
//                continue;
//            if ( node->isInUse() )
//                continue;

        	FD_SET( *iter, &readset );
        }

        // Setting time-out
        timeval timeout;
        timeout.tv_sec = g_READ_READY_INTERVAL;
        timeout.tv_usec = 0;

        int fd_max = *( m_socksToSelect.rbegin() );
        // TODO: Send errno to log
        int retval = ::select( fd_max + 1, &readset, NULL, NULL, &timeout );
        if ( retval < 0 )
        {
            FaultLog( erError, "Server socket got error while calling \"select\": " + errno2str() );
            return;
        }

        if ( 0 == retval )
            return;

        // check whether a proof server tries to connect to proof workers
        iter = m_socksToSelect.begin();
        iter_end = m_socksToSelect.end();
        for ( ; iter != iter_end; ++iter )
        {
            // exclude a server socket
            if ( *iter == f_serverSocket )
                continue;

            if ( FD_ISSET( *iter, &readset ) )
            {
                CNodeContainer::node_type node = m_nodes.getNode( *iter );
                if ( node.get() == NULL )
                    continue; // TODO: Log me!

                if ( !node->isActive() )
                {
                    // if yes, then we need to activate this node and
                    // add it to the packetforwarder
                    int fd = accept( *iter, NULL, NULL );
                    if ( fd < 0 )
                    {
                        FaultLog( erError, "PROOF client emulator can't accept a connection: " + errno2str() );
                        continue;
                    }

                    // remove the node from the container
                    m_nodes.removeNode( *iter );
                    m_nodes.removeNode( node->first() );

                    // update the second socket fd in the container
                    // and activate the node
                    node->updateSecond( fd );
                    node->activate();

                    // add the updated node to the container
                    m_nodes.addNode( node );

                    // remove this socket from the list
                    // we don't need to monitor it anymore
                    m_socksToSelect.erase( iter++ );
                    // add both sockets to "select"
                    // these are proxy sockets for a packet forwarding
                    m_socksToSelect.insert( node->first() );
                    m_socksToSelect.insert( node->second() );
                }
                else
                {
                    // we get a task for packet forwarder
                  //  m_threadPool.pushTask( *iter, node.get() );
                	node->dealWithData( *iter );
                }
            }
        }

        // check whether agent's client tries to connect..
        if ( FD_ISSET( f_serverSocket, &readset ) )
        {
            inet::smart_socket socket( _server.Accept() );
            createClientNode( socket );
        }

    }

//=============================================================================
    void CAgentServer::createClientNode( MiscCommon::INet::smart_socket &_sock )
    {
        // checking protocol version
        string sReceive;
        inet::receive_string( _sock, &sReceive, g_nBUF_SIZE );
        DebugLog( erOK, "Server received: " + sReceive );

        // TODO: Implement protocol version check
        string sOK( g_szRESPONSE_OK );
        DebugLog( erOK, "Server sends: " + sOK );
        inet::send_string( _sock, sOK );

        // TODO: Receiving user name -- now we always assume that this is a user name -- WE NEED to implement a simple protocol!!!
        string sUsrName;
        receive_string( _sock, &sUsrName, g_nBUF_SIZE );
        DebugLog( erOK, "Server received: " + sUsrName );

        DebugLog( erOK, "Server sends: " + sOK );
        send_string( _sock, sOK );

        string strSocketInfo;
        inet::socket2string( _sock, &strSocketInfo );
        string strSocketPeerInfo;
        inet::peer2string( _sock, &strSocketPeerInfo );
        stringstream ss;
        ss
        << "Accepting connection on : " << strSocketInfo
        << " for peer: " << strSocketPeerInfo;
        InfoLog( erOK, ss.str() );

        const int port = inet::get_free_port( m_Data.m_agentLocalClientPortMin, m_Data.m_agentLocalClientPortMax );
        if ( 0 == port )
            throw runtime_error( "Can't find any free port from the given range." );

        // Add a worker to PROOF cfg
        string strRealWrkHost;
        inet::peer2string( _sock, &strRealWrkHost );

        // Update proof.cfg with active workers
        string strPROOFCfgString( createPROOFCfgEntryString( sUsrName, port, strRealWrkHost ) );

        // Now when we got a connection from our worker, we need to create a local server (for that worker)
        // which actually will emulate a local worker node for a proof server
        // Listening for PROOF master connections
        // Whenever he tries to connect to its clients we will catch it and redirect it
        inet::CSocketServer localPROOFclient;
        localPROOFclient.Bind( port );
        localPROOFclient.Listen( 1 );
        localPROOFclient.GetSocket().set_nonblock();

        // Then we add this node to a nodes container
        CNodeContainer::node_type node( new CNode( _sock.detach(), localPROOFclient.GetSocket().detach(), strPROOFCfgString ) );
        node->disable();
        m_nodes.addNode( node );
        // Update proof.cfg according to a current number of active workers
        updatePROOFCfg();

        // add new worker's localPROOFServer socket to the main "select"
        m_socksToSelect.insert( node->second() );
    }
//=============================================================================
    void CAgentServer::deleteServerInfoFile()
    {
        // TODO: check error code
        unlink( m_serverInfoFile.c_str() );
    }

//=============================================================================
    void CAgentServer::createPROOFCfg()
    {
        ofstream f( m_commonOptions.m_proofCFG.c_str() );

        // getting local host name
        string host;
        MiscCommon::get_hostname( &host );
        // master host name is the same for Server and Worker and equal to local host name
        stringstream ss;
        ss
        << "#master " << host << "\n"
        << "master " << host;

        m_masterEntryInPROOFCfg = ss.str();

        f << m_masterEntryInPROOFCfg << endl;

        //if ( pThis->GetMode() == Client )
        // {
        //     f_out << "worker " << host << " perf=100" << std::endl;
        // }
    }

//=============================================================================
    string CAgentServer::createPROOFCfgEntryString( const string &_UsrName,
                                                    unsigned short _Port, const string &_RealWrkHost )
    {
        stringstream ss;
        ss
        << "#worker " << _UsrName << "@" << _RealWrkHost << " (redirect through localhost:" << _Port << ")\n"
        << "worker " << _UsrName << "@localhost port="  << _Port << " perf=100" << endl;

        return ss.str();
    }

//=============================================================================
    void CAgentServer::updatePROOFCfg()
    {
        std::ofstream f( m_commonOptions.m_proofCFG.c_str() );
        if ( !f.is_open() )
            throw std::runtime_error( "Can't open the PROOF configuration file: " + m_commonOptions.m_proofCFG );

        // remove bad nodes
        m_nodes.removeBadNodes();

        const CNodeContainer::unique_container_type *const nodes = m_nodes.getNods();

        // a master host
        f << m_masterEntryInPROOFCfg << endl;

        // write entries to proof.cfg
        // proof workers
        CNodeContainer::unique_container_type::const_iterator iter = nodes->begin();
        CNodeContainer::unique_container_type::const_iterator iter_end = nodes->end();
        for ( ; iter != iter_end; ++iter )
        {
            f << ( *iter )->getPROOFCfgEntry() << endl;
        }
    }
}
