/************************************************************************/
/**
 * @file AgentServer.cpp
 * @brief
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2009-10-02
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
*************************************************************************/
#include "AgentServer.h"
// API
#include <csignal>
// BOOST
#include <boost/thread/mutex.hpp>
// MiscCommon
#include "ErrorCode.h"
#include "INet.h"
#include "SysHelper.h"
//=============================================================================
using namespace std;
using namespace PROOFAgent;
using namespace MiscCommon;
namespace inet = MiscCommon::INet;
//=============================================================================
extern sig_atomic_t graceful_quit;
// a monitoring thread timeout (in seconds)
const size_t g_monitorTimeout = 10;
//=============================================================================
struct is_bad_wrk
{
    bool operator()( const wrkValue_t &_val )
    {
        return ( 0 >= _val.first || _val.second.m_removeMe );
    }
};
//=============================================================================
CAgentServer::CAgentServer( const SOptions_t &_data ):
        CAgentBase( _data.m_podOptions.m_server.m_common ),
        m_threadPool( _data.m_podOptions.m_server.m_common.m_agentThreads, m_signalPipeName ),
        m_workerMaxID( 0 )
{
    m_Data = _data.m_podOptions.m_server;
    m_serverInfoFile = _data.m_serverInfoFile;
}
//=============================================================================
CAgentServer::~CAgentServer()
{
    deleteServerInfoFile();
}
//=============================================================================
void CAgentServer::monitor()
{
    while ( true )
    {
        // TODO: we need to check real PROOF port here (from cfg)
        if ( !IsPROOFReady( 0 ) || graceful_quit )
        {
            FaultLog( erError, "Can't connect to PROOF/XRD service." );
            graceful_quit = 1;
        }

        if ( m_idleWatch.isTimedout( m_Data.m_common.m_shutdownIfIdleForSec ) )
        {
            InfoLog( "Agent's idle time has just reached a defined maximum. Exiting..." );
            graceful_quit = 1;
        }

        if ( graceful_quit )
        {
            // wake up (from "select") the main thread, so that it can update it self
            if ( write( m_fdSignalPipe, "1", 1 ) < 0 )
                FaultLog( erError, "Can't signal to the main thread via a named pipe: " + errno2str() );

            return;
        }

        sleep( g_monitorTimeout );
    }
}
//=============================================================================
void CAgentServer::run()
{
    try
    {
        createPROOFCfg();

        readServerInfoFile( m_serverInfoFile );

        inet::CSocketServer server;
        server.Bind( m_agentServerListenPort );
        server.Listen( 200 ); // TODO: Move this number of queued clients to config
        server.setNonBlock(); // Nonblocking server socket

        // Add main server's socket to the list of sockets to select
        f_serverSocket = server.getSocket();

        InfoLog( "Entering into the main \"select\" loop..." );
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
int CAgentServer::prepareFDSet( fd_set *_readset )
{
    FD_ZERO( _readset );

    // Set server FDs first
    FD_SET( f_serverSocket, _readset );
    FD_SET( m_fdSignalPipe, _readset );

    // Max value from of FDs. Used by the "select"
    int fd_max(( f_serverSocket > m_fdSignalPipe ) ? f_serverSocket : m_fdSignalPipe );

    bool need_update( false );
    workersMap_t::size_type s( m_adminConnections.size() );
    // remove bad admin connections
    m_adminConnections.remove_if( is_bad_wrk() );
    if ( s != m_adminConnections.size() )
        need_update = true;

    // adding all sockets which are on the admin channel
    workersMap_t::const_iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::const_iterator wrk_iter_end = m_adminConnections.end();
    for ( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        int fd = wrk_iter->first;
        FD_SET( fd, _readset );
        fd_max = fd > fd_max ? fd : fd_max;
    }

    // Now set all FDs of all of the nodes
    // TODO: implement poll or check that a number of sockets is not higher than 1024 (limitations of "select" )
    Sockets_type::iterator iter = m_socksToSelect.begin();
    Sockets_type::iterator iter_end = m_socksToSelect.end();
    for ( ; iter != iter_end; ++iter )
    {
        if ( NULL == *iter )
            continue;

        if ( !( *iter )->isValid() )
        {
            need_update = true;
            iter = m_socksToSelect.erase( iter );
            // erase returns a bidirectional iterator pointing to the new location of the
            // element that followed the last element erased by the function call.
            // The loop will increment the iterator, we therefore need to move one step back
            --iter;
            continue;
        }

        if ( !( *iter )->isInUse( CNode::nodeSocketFirst ) )
        {
            int fd = ( *iter )->getSocket( CNode::nodeSocketFirst );
            FD_SET( fd, _readset );
            fd_max = fd > fd_max ? fd : fd_max;
        }
        if ( !( *iter )->isInUse( CNode::nodeSocketSecond ) )
        {
            int fd = ( *iter )->getSocket( CNode::nodeSocketSecond );
            FD_SET( fd, _readset );
            fd_max = fd > fd_max ? fd : fd_max;
        }
    }

    // Updating nodes list and proof.cfg
    if ( need_update )
    {
        need_update = false;
        updatePROOFCfg();
    }

    return fd_max;
}
//=============================================================================
void CAgentServer::mainSelect( const inet::CSocketServer &_server )
{
    fd_set readset;
    int fd_max = prepareFDSet( &readset );

    // main "Select"
    int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );
    if ( retval < 0 )
    {
        FaultLog( erError, "Server socket got error while calling \"select\": " + errno2str() );
        return;
    }

    if ( 0 == retval )
        return;

    Sockets_type::iterator iter = m_socksToSelect.begin();
    Sockets_type::iterator iter_end = m_socksToSelect.end();
    for ( ; iter != iter_end; ++iter )
    {
        if ( *iter == NULL || !( *iter )->isValid() )
            continue; // TODO: Log me!

        if ( FD_ISSET(( *iter )->getSocket( CNode::nodeSocketFirst ), &readset ) )
        {
            // there is a read-ready socket
            // if the node is active, everything is fine, but
            // it could be a case when the node is not yet active
            // probably the worker has dropped a connection (in this case
            // socket is read-ready but has 0 bytes in the stream.).
            // we try to read from it in anyway. Further procedures will mark
            // it as a bad one.
            if ( !( *iter )->isActive() )
            {
                InfoLog( "An inactive remote worker is in the ready-to-read state. It could mean that it has just dropped the connection." );
            }

            // update the idle timer
            m_idleWatch.touch();

            // we get a task for packet forwarder
            if (( *iter )->isInUse( CNode::nodeSocketFirst ) )
                continue;
            m_threadPool.pushTask( CNode::nodeSocketFirst, iter->get() );
        }

        if ( FD_ISSET(( *iter )->getSocket( CNode::nodeSocketSecond ), &readset ) )
        {
            // check whether a proof server tries to connect to proof workers

            // update the idle timer
            m_idleWatch.touch();

            if ( !( *iter )->isActive() )
            {
                // if yes, then we need to activate this node and
                // add it to the packetforwarder
                int fd = accept(( *iter )->getSocket( CNode::nodeSocketSecond ), NULL, NULL );
                if ( fd < 0 )
                {
                    FaultLog( erError, "PROOF client emulator can't accept a connection: " + errno2str() );
                    continue;
                }

                // update the second socket fd in the container
                // and activate the node
                ( *iter )->update( fd, CNode::nodeSocketSecond );
                ( *iter )->activate();
            }
            else
            {
                // we get a task for packet forwarder
                if (( *iter )->isInUse( CNode::nodeSocketSecond ) )
                    continue;

                m_threadPool.pushTask( CNode::nodeSocketSecond, iter->get() );
            }
        }
    }

    // Admin connection should be checked after m_socksToSelect (packet forwarded PF connections),
    // because some of the admin connections could became a PF connection see createClientNode()
    // In this case the socket will be detected twice as a read-ready-socket.
    workersMap_t::iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::iterator wrk_iter_end = m_adminConnections.end();
    for ( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        if ( wrk_iter->first <= 0 )
            continue;

        if ( 0 == FD_ISSET( wrk_iter->first, &readset ) )
            continue;

        // update the idle timer
        m_idleWatch.touch();

        try
        {
            // Starting a server communication
            processAdminConnection( *wrk_iter );
        }
        catch ( exception & e )
        {
            WarningLog( 0, e.what() );
            continue;
        }
    }

    // check whether agent's client tries to connect..
    if ( FD_ISSET( f_serverSocket, &readset ) )
    {
        // update the idle timer
        m_idleWatch.touch();

        // accepting a new connection on the admin channel
        inet::smart_socket wrk( _server.Accept() );
        wrk.set_nonblock();
        m_adminConnections.push_back( workersMap_t::value_type( wrk.detach(), SWorkerInfo() ) );
    }

    // we got a signal for update
    // reading everything from the pipe and letting select update all of its FDs
    if ( FD_ISSET( m_fdSignalPipe, &readset ) )
    {
        const int read_size = 64;
        char buf[read_size];
        int numread( 0 );
        numread = read( m_fdSignalPipe, buf, read_size );
    }

}
//=============================================================================
void CAgentServer::sendServerRequest( workersMap_t::value_type &_wrk )
{
    while ( !_wrk.second.m_requests.empty() )
    {
        stringstream ss;
        ss
        << "sending a server request: ";

        switch ( _wrk.second.m_requests.front() )
        {
            case cmdGET_ID:
                // request client's id
                ss << "[get id].";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdGET_ID ) );
                break;
            case cmdGET_HOST_INFO:
                ss << "[get host info].";
                // request client's host information
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdGET_HOST_INFO ) );
                break;
            case cmdSHUTDOWN:
                ss << "[SHUT DOWN].";
                _wrk.second.m_removeMe = true;
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdSHUTDOWN ) );
                break;
            case cmdUSE_PACKETFORWARDING_PROOF:
                ss << "[use packet forwarder].";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdUSE_PACKETFORWARDING_PROOF ) );
                break;
            case cmdUSE_DIRECT_PROOF:
                ss << "[use a direct proof connection].";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdUSE_DIRECT_PROOF ) );
                break;
            default:
                ss << "[NO COMMAND].";
                WarningLog( 0, "unexpected command has been found in the server requests queue." );
                break;
        }
        _wrk.second.m_requests.pop();
        ss
        << " To host: " << _wrk.second.string();
        DebugLog( 0, ss.str() );
    }
}
//=============================================================================
void CAgentServer::processAdminConnection( workersMap_t::value_type &_wrk )
{
    while ( true )
    {
        // send queued requests
        sendServerRequest( _wrk );

        CProtocol::EStatus_t ret = _wrk.second.m_protocol.read( _wrk.first );
        switch ( ret )
        {
            case CProtocol::stDISCONNECT:
                {
                    stringstream ss;
                    ss << "Worker [" << _wrk.second.string() << "] has just dropped the connection";
                    InfoLog( ss.str() );
                    close( _wrk.first );
                    _wrk.first = -1;
                }
                break;
            case CProtocol::stAGAIN:
                // send queued requests
                sendServerRequest( _wrk );
                return;
            case CProtocol::stOK:
                {
                    BYTEVector_t data;
                    SMessageHeader header = _wrk.second.m_protocol.getMsg( &data );
                    switch ( static_cast<ECmdType>( header.m_cmd ) )
                    {
                        case cmdVERSION:
                            {
                                SVersionCmd v;
                                v.convertFromData( data );
                                // so far we require all versions to be the same
                                if ( v.m_version != _wrk.second.m_protocol.getVersion() )
                                {
                                    stringstream ss;
                                    ss
                                    << "Shutting the worker down: "
                                    << _wrk.second.string()
                                    << ". It has incompatible protocol version.";
                                    InfoLog( ss.str() );
                                    _wrk.second.m_requests.push( cmdSHUTDOWN );
                                }
                                // request client's id
                                // it could happen that worker has already ID.
                                // so, before assigning a new id we have to ask first
                                _wrk.second.m_requests.push( cmdGET_ID );
                                // request client's host information
                                _wrk.second.m_requests.push( cmdGET_HOST_INFO );
                            }
                            break;
                        case cmdID:
                            {
                                SIdCmd id;
                                id.convertFromData( data );
                                if ( 0 == id.m_id )
                                {
                                    // set an id to the worker
                                    ++m_workerMaxID;
                                    id.m_id = m_workerMaxID;
                                    data.clear();
                                    id.convertToData( &data );
                                    _wrk.second.m_protocol.write( _wrk.first, static_cast<uint16_t>( cmdSET_ID ), data );
                                }
                                _wrk.second.m_id = id.m_id;
                            }
                            break;
                        case cmdHOST_INFO:
                            {
                                SHostInfoCmd h;
                                h.convertFromData( data );
                                _wrk.second.m_host = h.m_host;
                                _wrk.second.m_user = h.m_username;
                                _wrk.second.m_proofPort = h.m_proofPort;

                                setupPROOFWorker( _wrk );
                                stringstream ss;
                                ss << "Server received client's host info: " << h;
                                DebugLog( 0, ss.str() );
                            }
                            break;
                        default:
                            WarningLog( 0, "Unexpected message in the admin channel" );
                            break;

                    }
                    break;
                }
        }
    }
    // send queued requests
    sendServerRequest( _wrk );
}
//=============================================================================
void CAgentServer::usePacketForwarding( workersMap_t::value_type &_wrk )
{
    _wrk.second.m_requests.push( cmdUSE_PACKETFORWARDING_PROOF );
    createClientNode( _wrk );
}
//=============================================================================
void CAgentServer::setupPROOFWorker( workersMap_t::value_type &_wrk )
{
    if ( m_Data.m_packetForwarding == "yes" )
    {
        // use packet forwarder
        usePacketForwarding( _wrk );
        return;
    }

    // in case when m_Data.m_packetForwarding is "no" or "auto"
    // check whether a direct connection to a xproof worker possible
    try
    {
        inet::CSocketClient c;
        c.connect( _wrk.second.m_proofPort, _wrk.second.m_host );
    }
    catch ( ... ) // we got a problem to connect to a worker
    {
        if ( m_Data.m_packetForwarding == "no" )
        {
            stringstream ss;
            ss
            << "Shutting the worker down: "
            << _wrk.second.string()
            << ". User requested a direct PROOF connection, which is not possible for this host.";
            InfoLog( ss.str() );
            _wrk.second.m_requests.push( cmdSHUTDOWN );
            return;
        }

        // use packet forwarder
        usePacketForwarding( _wrk );
        return;
    }

    // using a direct connection to xproof
    _wrk.second.m_requests.push( cmdUSE_DIRECT_PROOF );

    // Update proof.cfg with active workers
    _wrk.second.m_proofCfgEntry =
        createPROOFCfgEntryString( _wrk.second.m_user, _wrk.second.m_proofPort, _wrk.second.m_host, false );
    // Update proof.cfg according to a current number of active workers
    updatePROOFCfg();

    stringstream ss;
    ss
    << "Using a direct connection to xproof for: "
    << _wrk.second.string();
    InfoLog( ss.str() );
}
//=============================================================================
void CAgentServer::createClientNode( workersMap_t::value_type &_wrk )
{
    // marking this worker to be removed from the admin channel
    // TODO: in the future versions, workers will use admin channel all the time in all modes
    _wrk.second.m_removeMe = true;

    stringstream ss;
    ss
    << "Using a packet forwarding for the worker: "
    << _wrk.second.string();
    InfoLog( erOK, ss.str() );

    const int port = inet::get_free_port( m_Data.m_agentLocalClientPortMin, m_Data.m_agentLocalClientPortMax );
    if ( 0 == port )
        throw runtime_error( "Can't find any free port from the given range." );

    // Add a worker to PROOF cfg
    string strRealWrkHost;
    inet::peer2string( _wrk.first, &strRealWrkHost );

    // Update proof.cfg with active workers
    string strPROOFCfgString( createPROOFCfgEntryString( _wrk.second.m_user, port, strRealWrkHost, true ) );

    // Now when we got a connection from our worker, we need to create a local server (for that worker)
    // which actually will emulate a local worker node for a proof server
    // Listening for PROOF master connections
    // Whenever he tries to connect to its clients we will catch that and redirect it
    inet::CSocketServer localPROOFclient;
    localPROOFclient.Bind( port );
    localPROOFclient.Listen( 1 );
    localPROOFclient.setNonBlock();

    // Then we add this node to a nodes container
    node_type node(
        new CNode( _wrk.first, localPROOFclient.detach(),
                   strPROOFCfgString, m_Data.m_common.m_agentNodeReadBuffer ) );
    node->disable();
    // add new worker's sockets to the main "select"
    m_socksToSelect.push_back( node );
    // Update proof.cfg according to a current number of active workers
    updatePROOFCfg();
}
//=============================================================================
void CAgentServer::deleteServerInfoFile() const
{
    // TODO: check error code
    unlink( m_serverInfoFile.c_str() );
}
//=============================================================================
void CAgentServer::createPROOFCfg()
{
    DebugLog( erOK, "Creating a PROOF configuration file..." );

    ofstream f( m_commonOptions.m_proofCFG.c_str() );
    if ( !f.is_open() )
        throw runtime_error( "can't open " + m_commonOptions.m_proofCFG + " for writing." );

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
}
//=============================================================================
string CAgentServer::createPROOFCfgEntryString( const string &_UsrName,
                                                unsigned short _Port,
                                                const string &_RealWrkHost,
                                                bool usePF )
{
    stringstream ss;
    if ( usePF )
    {
        ss
        << "#worker " << _UsrName << "@" << _RealWrkHost << " (packet forwarder: localhost:" << _Port << ")\n"
        << "worker " << _UsrName << "@localhost port="  << _Port << " perf=100" << endl;
    }
    else
    {
        ss
        << "#worker " << _UsrName << "@" << _RealWrkHost << ":" << _Port << " (direct)\n"
        << "worker " << _UsrName << "@" << _RealWrkHost << " port="  << _Port << " perf=100" << endl;
    }
    return ss.str();
}
//=============================================================================
void CAgentServer::updatePROOFCfg()
{
    std::ofstream f( m_commonOptions.m_proofCFG.c_str() );
    if ( !f.is_open() )
        throw std::runtime_error( "Can't open the PROOF configuration file: " + m_commonOptions.m_proofCFG );

    // a master host
    f << m_masterEntryInPROOFCfg << endl;

    // write entries to proof.cfg
    // proof workers
    Sockets_type::const_iterator iter = m_socksToSelect.begin();
    Sockets_type::const_iterator iter_end = m_socksToSelect.end();
    for ( ; iter != iter_end; ++iter )
    {
        f << ( *iter )->getPROOFCfgEntry() << endl;
    }

    // a directly connected workers
    workersMap_t::iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::iterator wrk_iter_end = m_adminConnections.end();
    for ( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        if ( 0 >= wrk_iter->first )
            continue;

        if ( wrk_iter->second.m_proofCfgEntry.empty() )
            continue;

        f << wrk_iter->second.m_proofCfgEntry << endl;
    }
}
