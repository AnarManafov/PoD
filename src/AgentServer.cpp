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
#include "SysHelper.h"
// PROOFAgent
#include "AgentServer.h"
//=============================================================================
#define SIGNAL_PIPE_PATH        "$POD_LOCATION/etc/signal_pipe"
//=============================================================================
using namespace std;
using namespace MiscCommon;
namespace inet = MiscCommon::INet;
//=============================================================================
extern sig_atomic_t graceful_quit;

const size_t g_monitorTimeout = 5; // in seconds

// TODO: Move to config or make it autodetectable...
const size_t g_numThreads = 8;
//=============================================================================
namespace PROOFAgent
{

//=============================================================================
    CAgentServer::CAgentServer( const SOptions_t &_data ):
            CAgentBase( _data.m_podOptions.m_server.m_common ),
            m_threadPool( g_numThreads, SIGNAL_PIPE_PATH ),
            m_fdSignalPipe( 0 )
    {
        m_Data = _data.m_podOptions.m_server;
        m_serverInfoFile = _data.m_serverInfoFile;

        //InfoLog( MiscCommon::erOK, "Agent Server configuration:" ) << m_Data;

        // create a named pipe (our signal pipe)
        // it's use to interrupt "select" and give a chance to new sockets to be added
        // to the "select"
        DebugLog( erOK, "Creating a communication pipe for the thread pool..." );
        string path( SIGNAL_PIPE_PATH );
        smart_path( &path );
        int ret_val = mkfifo( path.c_str(), 0666 );

        if (( ret_val == -1 ) && ( errno != EEXIST ) )
        {
            FaultLog( erError, "Error creating the named pipe: + path" );
            graceful_quit = 1;
        }

        /* Open the pipe for reading */
        m_fdSignalPipe = open( path.c_str(), O_RDWR | O_NONBLOCK );
    }

//=============================================================================
    CAgentServer::~CAgentServer()
    {
        deleteServerInfoFile();
        close( m_fdSignalPipe );
    }

//=============================================================================
    void CAgentServer::run()
    {
        DebugLog( erOK, "Creating a PROOF configuration file..." );
        createPROOFCfg();

        try
        {
            readServerInfoFile( m_serverInfoFile );

            inet::CSocketServer server;
            server.Bind( m_agentServerListenPort );
            server.Listen( 200 ); // TODO: Move this number of queued clients to config
            server.GetSocket().set_nonblock(); // Nonblocking server socket

            // Add main server's socket to the list of sockets to select
            f_serverSocket = server.GetSocket().get();
            m_socksToSelect.push_back( f_serverSocket );
            m_socksToSelect.push_back( m_fdSignalPipe );
            DebugLog( erOK, "Entering into the \"select\" loop..." );
            while ( true )
            {
                // Checking whether signal has arrived
                if ( graceful_quit )
                {
                    InfoLog( erOK, "STOP signal received." );
                    m_threadPool.stop();
                    return ;
                }

                // ------------------------
                // A Global "select"
                // ------------------------�
                mainSelect( server );
            }
        }
        catch ( exception & e )
        {
            FaultLog( erError, e.what() );
        }
    }

//=============================================================================
    void CAgentServer::monitor()
    {
        while ( true )
        {
            // TODO: we need to check real PROOF port here (from cfg)
            if ( !IsPROOFReady( 0 ) )
            {
                FaultLog( erError, "Can't connect to PROOF/XRD service." );
                graceful_quit = 1;

                // wake up (from "select") the main thread, so that it can update it self
                if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
                    FaultLog( erError, "Can't signal to the main thread via a named pipe: " + errno2str() );

                return;
            }

            sleep( g_monitorTimeout );
        }
    }

//=============================================================================
    void CAgentServer::mainSelect( const inet::CSocketServer &_server )
    {
        fd_set readset;
        FD_ZERO( &readset );
        int fd_max( -1 );

        // TODO: implement poll or check that a number of sockets is not higher than 1024 (limitations of "select" )
        bool need_update( false );
        Sockets_type::iterator iter = m_socksToSelect.begin();
        Sockets_type::iterator iter_end = m_socksToSelect.end();
        for ( ; iter != iter_end; ++iter )
        {
            // don't include node which are being processed at this moment
            if ( *iter != f_serverSocket && *iter != m_fdSignalPipe )
            {
                CNodeContainer::node_type node = m_nodes.getNode( *iter );
                if ( node.get() == NULL || node->isInUse() )
                {
                    continue;
                }
                if ( !node->isValid() )
                {
                    InfoLog( erOK, "Found a bad worker, removing: " + node->getPROOFCfgEntry() );
                    need_update = true;
                    iter = m_socksToSelect.erase( iter );
                    // erase returns a bidirectional iterator pointing to the new location of the
                    // element that followed the last element erased by the function call.
                    // The loop will increment the iterator, we therefore need to move one step back
                    --iter;
                    continue;
                }
            }

            // looking for the highest fd
            if ( *iter > fd_max )
                fd_max = *iter;

            FD_SET( *iter, &readset );
        }

        // Updating nodes list and proof.cfg
        if ( need_update )
        {
            need_update = false;
            updatePROOFCfg();
        }

        // main "Select"
        int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );
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
            if ( *iter == f_serverSocket || *iter == m_fdSignalPipe )
                continue;

            if ( FD_ISSET( *iter, &readset ) )
            {
                CNodeContainer::node_type node = m_nodes.getNode( *iter );
                if ( node.get() == NULL || !node->isValid() )
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
                    iter = m_socksToSelect.erase( iter );
                    --iter;
                    // add both sockets to "select"
                    // these are proxy sockets for a packet forwarding
                    m_socksToSelect.insert( iter, node->first() );
                    m_socksToSelect.insert( iter, node->second() );
                }
                else
                {
                    // we get a task for packet forwarder
                    if ( node->isInUse() )
                        continue;
                    node->setInUse( true );
                    m_threadPool.pushTask( *iter, node.get() );
                }
            }
        }

        // check whether agent's client tries to connect..
        if ( FD_ISSET( f_serverSocket, &readset ) )
        {
            inet::smart_socket socket( _server.Accept() );
            createClientNode( socket );
        }

        // we got a signal for update
        // reading everything from the pipe and letting select update all of its FDs
        if ( FD_ISSET( m_fdSignalPipe, &readset ) )
        {
            const int read_size = 20;
            char buf[read_size];
            int numread( 0 );
            do
            {
                numread = read( m_fdSignalPipe, buf, read_size );
            }
            while ( numread > 0 );
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
        m_socksToSelect.push_back( node->second() );
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
