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

        Copyright (c) 2009-2011 GSI, Scientific Computing devision. All rights reserved.
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
//  pod-agent
#include "version.h"
//=============================================================================
using namespace std;
using namespace PROOFAgent;
using namespace MiscCommon;
namespace inet = MiscCommon::INet;
//=============================================================================
extern sig_atomic_t graceful_quit;
// a monitoring thread timeout (in seconds)
const size_t g_monitorTimeout = 10;
// will be concatenated with (m_Data.m_common.m_workDir + "/")
const char *const g_xpdCFG = "etc/xpd.cf";
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
    m_threadPool( _data.m_podOptions.m_server.m_agentThreads, m_signalPipeName ),
    m_workerMaxID( 0 ),
    m_agentServerListenPort( 0 )
{
    m_Data = _data.m_podOptions.m_server;
    m_serverInfoFile = _data.m_serverInfoFile;

    string xpd( m_Data.m_common.m_workDir );
    smart_append( &xpd, '/' );
    xpd += g_xpdCFG;
    smart_path( &xpd );
    if( !m_proofStatus.readAdminPath( xpd, adminp_server ) )
    {
        string msg( "Can't find xproofd config: " );
        msg += xpd;
        FaultLog( 0, msg );
    }
    m_xpdPort = m_proofStatus.xpdPort();
    m_xpdPid = m_proofStatus.xpdPid();
    stringstream ss;
    ss << "Detected xpd [" << m_xpdPid << "] on port " << m_xpdPort;
    InfoLog( ss.str() );
}
//=============================================================================
CAgentServer::~CAgentServer()
{
    // Shut down all WNs in admin. channel
    shutdownWNs();

    deleteServerInfoFile();

    // Stop PoD server
    // This will stop all other server's processes as well
    if( fork() == 0 )
    {
        // invoking a new bash process can in some case overwrite env. vars
        // To be shure that our env is there, we call PoD_env.sh
        string cmd_env( "$POD_LOCATION/PoD_env.sh" );
        smart_path( &cmd_env );
        string cmd( "$POD_LOCATION/bin/pod-server stop" );
        smart_path( &cmd );
        string arg( "source " );
        arg += cmd_env;
        arg += " ; ";
        arg += cmd;
        execl( "/bin/bash", "bash", "-c", arg.c_str(), NULL );
    }
}
//=============================================================================
void CAgentServer::monitor()
{
    while( true )
    {
        if( !IsPROOFReady() || graceful_quit )
        {
            FaultLog( erError, "Can't connect to PROOF service." );
            graceful_quit = 1;
        }

        static uint16_t count = 0;
        if( count < 3 )
            ++count;
        // check status files of the proof
        // do that when at least one connection is direct
        // To simplify the things, we assume that number of admin connection is
        // equal to a number of direct PROOF connections.
        // NOTE: Call this check every third time or something, in order
        // to avoid resource overloading.
        if( !m_adminConnections.empty() && 3 == count )
        {
            updateIdle();
            count = 0;
        }

        if( m_idleWatch.isTimedout( m_Data.m_common.m_shutdownIfIdleForSec ) )
        {
            InfoLog( "Agent's idle time has just reached a defined maximum. Exiting..." );
            graceful_quit = 1;
        }

        if( graceful_quit )
        {
            // wake up (from 'select') the main thread, so that it can update it self
            if( write( m_fdSignalPipe, "1", 1 ) < 0 )
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

        // find a free port to listen on
        m_agentServerListenPort = inet::get_free_port( m_Data.m_agentPortsRangeMin, m_Data.m_agentPortsRangeMax );
        if( 0 == m_agentServerListenPort )
        {
            stringstream ss;
            ss
                    << "Can't find any free port from the given range: "
                    << m_Data.m_agentPortsRangeMin << ":" << m_Data.m_agentPortsRangeMax;
            throw runtime_error( ss.str() );
        }

        inet::CSocketServer server;
        server.Bind( m_agentServerListenPort );
        // a number of queued clients to config
        server.Listen( 200 );
        server.setNonBlock(); // Nonblocking server socket

        // Add main server's socket to the list of sockets to select
        f_serverSocket = server.getSocket();

        createServerInfoFile();

        InfoLog( "Entering into the main 'select' loop..." );
        while( true )
        {
            // Checking whether signal has arrived
            if( graceful_quit )
            {
                InfoLog( erOK, "STOP signal received." );
                m_threadPool.stop();
                return ;
            }

            // ------------------------
            // A Global 'select'
            // ------------------------ 
            mainSelect( server );
        }
    }
    catch( exception & e )
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
    if( s != m_adminConnections.size() )
        need_update = true;

    // adding all sockets which are on the admin channel
    workersMap_t::const_iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::const_iterator wrk_iter_end = m_adminConnections.end();
    for( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        int fd = wrk_iter->first;
        FD_SET( fd, _readset );
        fd_max = fd > fd_max ? fd : fd_max;
    }

    // Now set all FDs of all of the nodes
    // TODO: implement poll or check that a number of sockets is not higher than 1024 (limitations of "select" )
    Sockets_type::iterator iter = m_socksToSelect.begin();
    Sockets_type::iterator iter_end = m_socksToSelect.end();
    for( ; iter != iter_end; ++iter )
    {
        if( NULL == *iter )
            continue;

        if( !( *iter )->isValid() )
        {
            need_update = true;
            iter = m_socksToSelect.erase( iter );
            // erase returns a bidirectional iterator pointing to the new location of the
            // element that followed the last element erased by the function call.
            // The loop will increment the iterator, we therefore need to move one step back
            --iter;
            continue;
        }

        if( !( *iter )->isInUse( CNode::nodeSocketFirst ) )
        {
            int fd = ( *iter )->getSocket( CNode::nodeSocketFirst );
            FD_SET( fd, _readset );
            fd_max = fd > fd_max ? fd : fd_max;
        }
        if( !( *iter )->isInUse( CNode::nodeSocketSecond ) )
        {
            int fd = ( *iter )->getSocket( CNode::nodeSocketSecond );
            FD_SET( fd, _readset );
            fd_max = fd > fd_max ? fd : fd_max;
        }
    }

    // Updating nodes list and proof.cfg
    if( need_update )
    {
        need_update = false;
        updatePROOFCfg();
    }

    return fd_max;
}
//=============================================================================
// memberof to silence doxygen warning:
// warning: no matching class member found for
// This happens because doxygen is not handling namespaces in arguments properly
/**
 * @memberof PROOFAgent::CAgentServer
 *
 */
void CAgentServer::mainSelect( const inet::CSocketServer &_server )
{
    fd_set readset;
    int fd_max = prepareFDSet( &readset );

    // main "Select"
    int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );
    if( retval < 0 )
    {
        if( EINTR == errno )
        {
            InfoLog( "The main 'select' routine was interrupted by a signal." );
            return;
        }

        FaultLog( erError, "Server socket got error while calling 'select': " + errno2str() );
        return;
    }

    if( 0 == retval )
        return;

    Sockets_type::iterator iter = m_socksToSelect.begin();
    Sockets_type::iterator iter_end = m_socksToSelect.end();
    for( ; iter != iter_end; ++iter )
    {
        if( *iter == NULL || !( *iter )->isValid() )
            continue; // TODO: Log me!

        if( FD_ISSET(( *iter )->getSocket( CNode::nodeSocketFirst ), &readset ) )
        {
            // there is a read-ready socket
            // if the node is active, everything is fine, but
            // it could be a case when the node is not yet active
            // probably the worker has dropped a connection (in this case
            // socket is read-ready but has 0 bytes in the stream.).
            // we try to read from it in anyway. Further procedures will mark
            // it as a bad one.
            if( !( *iter )->isActive() )
            {
                InfoLog( "An inactive remote worker is in the ready-to-read state. It could mean that it has just dropped the connection." );
            }

            // update the idle timer
            m_idleWatch.touch();

            // we get a task for packet forwarder
            if(( *iter )->isInUse( CNode::nodeSocketFirst ) )
                continue;
            m_threadPool.pushTask( CNode::nodeSocketFirst, iter->get() );
        }

        if( FD_ISSET(( *iter )->getSocket( CNode::nodeSocketSecond ), &readset ) )
        {
            // check whether a proof server tries to connect to proof workers

            // update the idle timer
            m_idleWatch.touch();

            if( !( *iter )->isActive() )
            {
                // if yes, then we need to activate this node and
                // add it to the packetforwarder
                int fd = accept(( *iter )->getSocket( CNode::nodeSocketSecond ), NULL, NULL );
                if( fd < 0 )
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
                if(( *iter )->isInUse( CNode::nodeSocketSecond ) )
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
    for( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        if( wrk_iter->first <= 0 )
            continue;

        if( 0 == FD_ISSET( wrk_iter->first, &readset ) )
            continue;

        // update the idle timer
        m_idleWatch.touch();

        try
        {
            // Starting a server communication
            processAdminConnection( *wrk_iter );
        }
        catch( exception & e )
        {
            WarningLog( 0, e.what() );
            continue;
        }
    }

    // check whether agent's client tries to connect..
    if( FD_ISSET( f_serverSocket, &readset ) )
    {
        // update the idle timer
        m_idleWatch.touch();

        // accepting a new connection on the admin channel
        inet::smart_socket wrk( _server.Accept() );
        wrk.set_nonblock();
        string strRealWrkHost;
        inet::peer2string( wrk, &strRealWrkHost );
        m_adminConnections.push_back( workersMap_t::value_type( wrk.detach(), SWorkerInfo( strRealWrkHost ) ) );

    }

    // we got a signal for update
    // reading everything from the pipe and letting select to update all of its FDs
    if( FD_ISSET( m_fdSignalPipe, &readset ) )
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
    while( !_wrk.second.m_requests.empty() )
    {
        stringstream ss;
        ss
                << "sending request ";

        switch( _wrk.second.m_requests.front() )
        {
            case cmdGET_ID:
                // request client's id
                ss << "[get id]";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdGET_ID ) );
                break;
            case cmdGET_HOST_INFO:
                ss << "[get host info]";
                // request client's host information
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdGET_HOST_INFO ) );
                break;
            case cmdSHUTDOWN:
                ss << "[SHUT DOWN]";
                _wrk.second.m_removeMe = true;
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdSHUTDOWN ) );
                break;
            case cmdUSE_PACKETFORWARDING_PROOF:
                ss << "[use packet forwarder]";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdUSE_PACKETFORWARDING_PROOF ) );
                break;
            case cmdUSE_DIRECT_PROOF:
                ss << "[use a direct proof connection]";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdUSE_DIRECT_PROOF ) );
                break;
            case cmdGET_WRK_NUM:
                // request a number of PROOF workers
                ss << "[get a number of PROOF workers]";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdGET_WRK_NUM ) );
                break;
            case cmdUI_CONNECT_READY:
                // server is ready to accept UI requests
                ss << "[server is ready to accept UI requests]";
                _wrk.second.m_protocol.writeSimpleCmd( _wrk.first, static_cast<uint16_t>( cmdUI_CONNECT_READY ) );
                break;
            case cmdWNs_LIST:
                {
                    ss << "[sending a list of available workers]";
                    SWnListCmd lst;
                    string full;
                    string element;
                    // proof workers
                    Sockets_type::const_iterator iter = m_socksToSelect.begin();
                    Sockets_type::const_iterator iter_end = m_socksToSelect.end();
                    for( ; iter != iter_end; ++iter )
                    {
                        full = ( *iter )->getPROOFCfgEntry();
                        // extract only the comment part
                        size_t pos1 = full.find( '#' );
                        size_t pos2 = full.find( '\n', pos1 );
                        if( string::npos != pos1 && string::npos != pos2 )
                            element = full.substr( pos1 + 1, pos2 - pos1 - 1 );
                        lst.m_container.push_back( element );
                    }

                    // a directly connected workers
                    workersMap_t::iterator wrk_iter = m_adminConnections.begin();
                    workersMap_t::iterator wrk_iter_end = m_adminConnections.end();
                    for( ; wrk_iter != wrk_iter_end; ++wrk_iter )
                    {
                        if( 0 >= wrk_iter->first )
                            continue;

                        if( wrk_iter->second.m_proofCfgEntry.empty() )
                            continue;

                        full = wrk_iter->second.m_proofCfgEntry;
                        // extract only the comment part
                        size_t pos1( 0 );
                        while( string::npos != ( pos1 = full.find( '#', pos1 ) ) )
                        {
                            size_t pos2 = full.find( '\n', pos1 );
                            if( string::npos != pos1 && string::npos != pos2 )
                                element = full.substr( pos1 + 1, pos2 - pos1 - 1 );
                            lst.m_container.push_back( element );
                            ++pos1;
                        }
                    }
                    BYTEVector_t data_to_send;
                    lst.convertToData( &data_to_send );
                    _wrk.second.m_protocol.write( _wrk.first, static_cast<uint16_t>( cmdWNs_LIST ), data_to_send );
                }
                break;
            default:
                ss << "[NO COMMAND]";
                WarningLog( 0, "unexpected command has been found in the server requests queue." );
                break;
        }
        _wrk.second.m_requests.pop();
        ss
                << " to host: " << _wrk.second.string();
        DebugLog( 0, ss.str() );
    }
}
//=============================================================================
void CAgentServer::processAdminConnection( workersMap_t::value_type &_wrk )
{
    CProtocol::EStatus_t ret = _wrk.second.m_protocol.read( _wrk.first );
    switch( ret )
    {
        case CProtocol::stDISCONNECT:
            {
                stringstream ss;
                ss
                        << "Client "
                        << _wrk.second.string() << " has just dropped the connection";
                InfoLog( ss.str() );
                close( _wrk.first );
                _wrk.first = -1;
            }
            break;
        case CProtocol::stAGAIN:
        case CProtocol::stOK:
            {
                while( _wrk.second.m_protocol.checkoutNextMsg() )
                {
                    processProtocolMsgs( _wrk );
                }
            }
            break;
    }

    // send queued requests
    sendServerRequest( _wrk );
}
//=============================================================================
void CAgentServer::processProtocolMsgs( workersMap_t::value_type &_wrk )
{
    BYTEVector_t data;
    SMessageHeader header = _wrk.second.m_protocol.getMsg( &data );
    switch( static_cast<ECmdType>( header.m_cmd ) )
    {
        case cmdVERSION:
            {
                SVersionCmd v;
                v.convertFromData( data );
                // so far we require all versions to be the same
                if( v.m_version != g_protocolCommandsVersion )
                {
                    stringstream ss;
                    ss
                            << "Shutting the worker down: "
                            << _wrk.second.string()
                            << ". The client has an incompatible protocol version ( srv. "
                            << g_protocolCommandsVersion << " vs. clnt. "
                            << v.m_version << ")";
                    InfoLog( ss.str() );
                    _wrk.second.m_requests.push( cmdSHUTDOWN );
                    break;
                }
                // request client's id
                // it could happen that worker has already ID.
                // so, before assigning a new id we have to ask first
                _wrk.second.m_requests.push( cmdGET_ID );
                // request a number of PROOF workers
                _wrk.second.m_requests.push( cmdGET_WRK_NUM );
                // request client's host information
                _wrk.second.m_requests.push( cmdGET_HOST_INFO );
            }
            break;
        case cmdUI_CONNECT:
            {
                // This command indicates, that a PoD UI is trying to connect
                SVersionCmd v;
                v.convertFromData( data );
                // so far we require all versions to be the same
                if( v.m_version != g_protocolCommandsVersion )
                {
                    stringstream ss;
                    ss
                            << "Shutting the UI connection down: "
                            << _wrk.second.string()
                            << ". The client has an incompatible protocol version ( srv. "
                            << g_protocolCommandsVersion << " vs. clnt. "
                            << v.m_version << ")";
                    InfoLog( ss.str() );
                    _wrk.second.m_requests.push( cmdSHUTDOWN );
                    break;
                }
                // Server is ready to answer UI requests
                _wrk.second.m_requests.push( cmdUI_CONNECT_READY );
                stringstream ss_msg;
                ss_msg
                        << "Accepting the connetion from PoD UI: "
                        << _wrk.second.string();
                InfoLog( ss_msg.str() );
            }
            break;
        case cmdGET_HOST_INFO:
            {
                InfoLog( "Client requests host information." );
                SHostInfoCmd h;
                get_cuser_name( &h.m_username );
                get_hostname( &h.m_host );
                h.m_version = PROJECT_VERSION_STRING;
                h.m_PoDPath = "$POD_LOCATION";
                smart_path( &h.m_PoDPath );
                h.m_xpdPort = m_xpdPort;
                h.m_xpdPid = m_xpdPid;
                h.m_agentPort = m_agentServerListenPort;
                h.m_agentPid = getpid();
                BYTEVector_t data_to_send;
                h.convertToData( &data_to_send );
                _wrk.second.m_protocol.write( _wrk.first, static_cast<uint16_t>( cmdHOST_INFO ), data_to_send );
            }
            break;
        case cmdID:
            {
                SIdCmd id;
                id.convertFromData( data );
                if( 0 == id.m_id )
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
        case cmdWRK_NUM:
            {
                SIdCmd id;
                id.convertFromData( data );
                if( id.m_id > 0 )
                    _wrk.second.m_numberOfPROOFWorkers = id.m_id;
            }
            break;
        case cmdHOST_INFO:
            {
                SHostInfoCmd h;
                h.convertFromData( data );
                _wrk.second.m_host = h.m_host;
                _wrk.second.m_user = h.m_username;
                _wrk.second.m_xpdPort = h.m_xpdPort;

                stringstream ss;
                ss << "Server received client's host info: " << h;
                DebugLog( 0, ss.str() );

                setupPROOFWorker( _wrk );
            }
            break;
        case cmdGET_WNs_LIST:
            InfoLog( "Client requests a list of available workers." );
            _wrk.second.m_requests.push( cmdWNs_LIST );
            break;
        default:
            WarningLog( 0, "Unexpected message in the admin channel" );
            break;
    }
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
    if( m_Data.m_packetForwarding == "yes" )
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
        c.connect( _wrk.second.m_xpdPort, _wrk.second.m_host );
    }
    catch( ... )  // we got a problem to connect to a worker
    {
        if( m_Data.m_packetForwarding == "no" )
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
        createPROOFCfgEntryString( _wrk.second.m_user, _wrk.second.m_xpdPort,
                                   _wrk.second.m_host, false,
                                   _wrk.second.m_numberOfPROOFWorkers );
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
    if( 0 == port )
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
    // add new worker's sockets to the main 'select'
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

    string proofCfg( getPROOFCfg() );
    ofstream f( proofCfg.c_str() );
    if( !f.is_open() )
        throw runtime_error( "can't open " + proofCfg + " for writing." );

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
                                                bool _usePF,
                                                unsigned int _numberOfPROOFWorkers )
{
    string entryTmpl( m_Data.m_proofCfgEntryPattern );
    stringstream ss_port;
    ss_port << _Port;

    stringstream ss;
    if( _usePF )
    {
        ss
                << "#worker " << _UsrName << "@" << _RealWrkHost << " (packet forwarder: localhost:" << _Port << ")\n";

        replace<string>( &entryTmpl, "%user%", _UsrName );
        replace<string>( &entryTmpl, "%host%", "localhost" );
        replace<string>( &entryTmpl, "%port%", ss_port.str() );
        ss << entryTmpl;
    }
    else
    {
        replace<string>( &entryTmpl, "%user%", _UsrName );
        replace<string>( &entryTmpl, "%host%", _RealWrkHost );
        replace<string>( &entryTmpl, "%port%", ss_port.str() );

        for( size_t i = 0; i < _numberOfPROOFWorkers; ++i )
        {
            if( 0 != i ) // separate multiple entries
                ss << "\n";

            ss
                    << "#worker " << _UsrName << "@" << _RealWrkHost << ":" << _Port << " (direct connection)\n"
                    << entryTmpl;
        }
    }
    return ss.str();
}
//=============================================================================
void CAgentServer::updatePROOFCfg()
{
    string proofCfg( getPROOFCfg() );
    std::ofstream f( proofCfg.c_str() );
    if( !f.is_open() )
        throw std::runtime_error( "Can't open the PROOF configuration file: " + proofCfg );

    // a master host
    f << m_masterEntryInPROOFCfg << endl;

    // write entries to proof.cfg
    // proof workers
    Sockets_type::const_iterator iter = m_socksToSelect.begin();
    Sockets_type::const_iterator iter_end = m_socksToSelect.end();
    for( ; iter != iter_end; ++iter )
    {
        f << ( *iter )->getPROOFCfgEntry() << "\n";
    }

    // a directly connected workers
    workersMap_t::iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::iterator wrk_iter_end = m_adminConnections.end();
    for( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        if( 0 >= wrk_iter->first )
            continue;

        if( wrk_iter->second.m_proofCfgEntry.empty() )
            continue;

        f << wrk_iter->second.m_proofCfgEntry << "\n";
    }

    f.flush();
}
//=============================================================================
void CAgentServer::createServerInfoFile()
{
    ofstream f( m_serverInfoFile.c_str() );
    if( !f.is_open() || !f.good() )
    {
        string msg( "Could not open a server info configuration file: " );
        msg += m_serverInfoFile;
        throw runtime_error( msg );
    }

    string srvHost;
    get_hostname( &srvHost );

    f
            << "[server]\n"
            << "host=" << srvHost << "\n"
            << "port=" << m_agentServerListenPort << "\n"
            << endl;
}
//=============================================================================
void CAgentServer::shutdownWNs()
{
    InfoLog( "Sending a shut down signal to all WNs in admin. channel." );
    workersMap_t::iterator wrk_iter = m_adminConnections.begin();
    workersMap_t::iterator wrk_iter_end = m_adminConnections.end();
    for( ; wrk_iter != wrk_iter_end; ++wrk_iter )
    {
        if( wrk_iter->first <= 0 )
            continue;

        try
        {
            // send shut down signal
            wrk_iter->second.m_requests.push( cmdSHUTDOWN );
            sendServerRequest( *wrk_iter );
        }
        catch( exception & e )
        {
            WarningLog( 0, e.what() );
            continue;
        }
    }
}
