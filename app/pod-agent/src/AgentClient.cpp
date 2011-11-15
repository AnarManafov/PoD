/************************************************************************/
/**
 * @file AgentClient.cpp
 * @brief Implementation file of AgentServer and AgentClient
 * @author Anar Manafov A.Manafov@gsi.de
 */ /*

        version number:     $LastChangedRevision$
        created by:         Anar Manafov
                            2007-03-01
        last changed by:    $LastChangedBy$ $LastChangedDate$

        Copyright (c) 2007-2011 GSI, Scientific Computing division. All rights reserved.
*************************************************************************/
#include "AgentClient.h"
// STD
#include <csignal>
// pod-agent
#include "Node.h"
#include "PARes.h"
#include "Protocol.h"
#include "ProtocolCommands.h"
//=============================================================================
using namespace std;
using namespace MiscCommon;
using namespace PROOFAgent;
namespace inet = MiscCommon::INet;
namespace po = boost::program_options;
//=============================================================================
const size_t g_monitorTimeout = 10; // in seconds
extern sig_atomic_t graceful_quit;
const char *const g_xpdCFG = "$POD_LOCATION/xpd.cf";
//=============================================================================
CAgentClient::CAgentClient( const SOptions_t &_data ):
    CAgentBase( _data.m_podOptions.m_worker.m_common ),
    m_id( 0 ),
    m_isDirect( false ),
    m_agentServerListenPort( 0 )
{
    m_Data = _data.m_podOptions.m_worker;
    m_serverInfoFile = _data.m_serverInfoFile;
    m_numberOfPROOFWorkers = _data.m_numberOfPROOFWorkers;
    // if m_numberOfPROOFWorkers is 0, than user wants us to decide
    // automatically on how many workers to create.
    if( 0 == m_numberOfPROOFWorkers )
    {
        m_numberOfPROOFWorkers = getNCores();
    }

    string xpd( g_xpdCFG );
    smart_path( &xpd );
    if( !m_proofStatus.readAdminPath( xpd, adminp_worker ) )
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
CAgentClient::~CAgentClient()
{
}
//=============================================================================
void CAgentClient::processAdminConnection( int _serverSock )
{
    CProtocol protocol;
    SVersionCmd v;
    BYTEVector_t ver;
    v.convertToData( &ver );
    protocol.write( _serverSock, static_cast<uint16_t>( cmdVERSION ), ver );
    bool bProcessNoMore( false );
    while( !bProcessNoMore )
    {
        InfoLog( "waiting for server commands" );
        CProtocol::EStatus_t ret = protocol.read( _serverSock );
        switch( ret )
        {
            case CProtocol::stDISCONNECT:
                throw runtime_error( "a disconnect has been detected on the adminChannel" );
            case CProtocol::stAGAIN:
            case CProtocol::stOK:
                {
                    while( protocol.checkoutNextMsg() )
                    {
                        bProcessNoMore = ( processProtocolMsgs( _serverSock, &protocol ) > 0 );
                        if( bProcessNoMore )
                            break;
                    }
                }
                break;
        }
    }
}
//=============================================================================
int CAgentClient::processProtocolMsgs( int _serverSock, CProtocol * _protocol )
{
    BYTEVector_t data;
    SMessageHeader header = _protocol->getMsg( &data );
    stringstream ss;
    ss << "CMD: " <<  header.m_cmd;
    InfoLog( ss.str() );
    switch( static_cast<ECmdType>( header.m_cmd ) )
    {
            //case cmdVERSION_BAD:
            //    break;
        case cmdGET_HOST_INFO:
            {
                InfoLog( "Server requests host information." );
                SHostInfoCmd h;
                get_cuser_name( &h.m_username );
                get_hostname( &h.m_host );
                h.m_xpdPort = m_xpdPort;
                BYTEVector_t data_to_send;
                h.convertToData( &data_to_send );
                _protocol->write( _serverSock, static_cast<uint16_t>( cmdHOST_INFO ), data_to_send );
            }
            break;
        case cmdGET_ID:
            {
                SIdCmd id;
                id.m_id = m_id;
                BYTEVector_t data_to_send;
                id.convertToData( &data_to_send );
                _protocol->write( _serverSock, static_cast<uint16_t>( cmdID ), data_to_send );
            }
            break;
        case cmdSET_ID:
            {
                SIdCmd id;
                id.convertFromData( data );
                m_id = id.m_id;
                stringstream ss;
                ss << "Server has assigned ID = " << m_id << " to this worker.";
                InfoLog( ss.str() );
            }
            break;
        case cmdUSE_PACKETFORWARDING_PROOF:
            // going out of the admin channel and start the packet forwarding
            InfoLog( "Server requests to use a packet forwarding for PROOF packages." );
            m_isDirect = false;
            return 1;
        case cmdUSE_DIRECT_PROOF:
            // TODO: we keep admin channel open and start the monitoring (proof status) thread
            m_isDirect = true;
            InfoLog( "Server requests to use a direct connection for PROOF packages." );
            break;
        case cmdGET_WRK_NUM:
            {
                // reuse SIdCmd
                SIdCmd wn_num;
                wn_num.m_id = m_numberOfPROOFWorkers;
                BYTEVector_t data_to_send;
                wn_num.convertToData( &data_to_send );
                _protocol->write( _serverSock, static_cast<uint16_t>( cmdWRK_NUM ), data_to_send );
                stringstream ss;
                ss << "A number of PROOF workers [" << m_numberOfPROOFWorkers << "] has been sent to server.";
                InfoLog( ss.str() );
            }
            break;
        case cmdSHUTDOWN:
            InfoLog( "Server requests to shut down..." );
            graceful_quit = true;
            throw runtime_error( "stop admin channel." );
        default:
            WarningLog( 0, "Unexpected message in the admin channel" );
            break;
    }
    return 0;
}
//=============================================================================
void CAgentClient::readServerInfoFile( const string &_filename )
{
    po::variables_map keys;
    po::options_description options(
        "Agent's server info config" );
    // HACK: Don't make a long add_options, otherwise Eclipse 3.5's CDT indexer can't handle it
    options.add_options()
    ( "server.host", po::value<string>(), "" )
    ( "server.port", po::value<unsigned int>(), "" )
    ;

    ifstream ifs( _filename.c_str() );
    if( !ifs.is_open() || !ifs.good() )
    {
        string msg( "Could not open a server info configuration file: " );
        msg += _filename;
        throw runtime_error( msg );
    }
    // Parse the config file
    po::store( po::parse_config_file( ifs, options ), keys );
    po::notify( keys );
    if( keys.count( "server.host" ) )
        m_agentServerHost = keys["server.host"].as<string> ();
    if( keys.count( "server.port" ) )
        m_agentServerListenPort = keys["server.port"].as<unsigned int> ();
}
//=============================================================================
void CAgentClient::run()
{
    try
    {
        createPROOFCfg();

        while( true )
        {
            if( graceful_quit )
            {
                InfoLog( "shutting Agent's instance down..." );
                return;
            }

            readServerInfoFile( m_serverInfoFile );

            InfoLog( "looking for PROOFAgent server to connect..." );
            // a connection to a Agent's server
            inet::CSocketClient client;
            client.connect( m_agentServerListenPort, m_agentServerHost );
            InfoLog( "Connection to the server established. Entering to admin channel." );

            try
            {
                // Starting a server communication
                processAdminConnection( client.getSocket() );
            }
            catch( exception & e )
            {
                WarningLog( 0, e.what() );
                continue;
            }


            InfoLog( "Entering into the main \"select\" loop..." );
            try
            {
                client.setNonBlock();
                // waiting until Agent server sends something
                // that would mean that a user initializes a PROOF session
                waitForServerToConnect( client.getSocket() );

                // now we need to connect to a local PROOF worker
                MiscCommon::INet::Socket_t proof_socket = connectToLocalPROOF( m_xpdPort );
                CNode node( client.detach(), proof_socket,
                            "", m_Data.m_common.m_agentNodeReadBuffer );

                // now we are ready to proxy all packages
                mainSelect( &node );
            }
            catch( exception & e )
            {
                WarningLog( erError, e.what() );
                continue;
            }
        }
    }
    catch( exception & e )
    {
        FaultLog( erError, e.what() );
    }
}

//=============================================================================
void CAgentClient::monitor()
{
    while( true )
    {
        // TODO: we need to check real PROOF port here (from cfg)
        if( !IsPROOFReady() )
        {
            FaultLog( erError, "Can't connect to PROOF service." );
            graceful_quit = 1;
        }

        static uint16_t count = 0;
        if( count < 3 )
            ++count;
        // check status files of the proof
        // do that when at least one connection is direct
        // NOTE: Call this check every third time or something, in order
        // to avoid resource overloading.
        if( m_isDirect && 3 == count )
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
            // wake up (from "select") the main thread, so that it can update it self
            if( write( m_fdSignalPipe, "1", 1 ) < 0 )
                FaultLog( erError, "Can't signal to the main thread via a named pipe: " + errno2str() );

            return;
        }

        sleep( g_monitorTimeout );
    }
}

//=============================================================================
void CAgentClient::waitForServerToConnect( MiscCommon::INet::Socket_t _sockToWait )
{
    InfoLog( "waiting for Agent's server to initialize a redirection procedure..." );

    fd_set readset;
    FD_ZERO( &readset );
    FD_SET( _sockToWait, &readset );
    // setting a signal pipe as well
    // we want to be interrupted
    FD_SET( m_fdSignalPipe, &readset );

    int retval = ::select(( _sockToWait > m_fdSignalPipe ? _sockToWait : m_fdSignalPipe ) + 1,
                          &readset, NULL, NULL, NULL );
    if( retval < 0 )
        throw system_error( "Error occurred while waiting for Agent server:" );

    // must actually never happen
    if( 0 == retval )
        throw system_error( "Select has timeout while waiting for an Agent server." );

    // we got a signal for update
    // reading everything from the pipe and exiting from the function
    if( FD_ISSET( m_fdSignalPipe, &readset ) )
    {
        const int read_size = 20;
        char buf[read_size];
        int numread( 0 );
        do
        {
            numread = read( m_fdSignalPipe, buf, read_size );
        }
        while( numread > 0 );

        throw runtime_error( "Got a wake up signal from the signal pipe. Stopping the main select..." );
    }

    m_idleWatch.touch();

    // if there were no exception raised we can process further
    InfoLog( "done waiting for Agent's server. Initializing a redirection procedure..." );
}

//=============================================================================
MiscCommon::INet::Socket_t CAgentClient::connectToLocalPROOF( unsigned int _proofPort )
{
    InfoLog( "Connecting to a local PROOF worker..." );

    // Connecting to the local client (a proof slave)
    inet::CSocketClient proof_client;
    proof_client.connect( _proofPort, "127.0.0.1" );
    InfoLog( "connected to the local proof service" );

    proof_client.setNonBlock();

    return proof_client.detach();
}

//=============================================================================
void CAgentClient::mainSelect( CNode *_node )
{
    while( true )
    {
        m_idleWatch.touch();

        // Checking whether signal has arrived
        if( graceful_quit )
        {
            InfoLog( erOK, "STOP signal received." );
            return;
        }


        fd_set readset;
        FD_ZERO( &readset );

        if( !_node || !_node->isValid() )
            return;

        int fd_first = _node->getSocket( CNode::nodeSocketFirst );
        int fd_second = _node->getSocket( CNode::nodeSocketSecond );
        FD_SET( fd_first, &readset );
        FD_SET( fd_second, &readset );
        int fd_max( fd_first > fd_second ? fd_first : fd_second );
        // setting a signal pipe as well
        // we want to be interrupted
        FD_SET( m_fdSignalPipe, &readset );
        fd_max = fd_max > m_fdSignalPipe ? fd_max : m_fdSignalPipe;

        int retval = ::select( fd_max + 1, &readset, NULL, NULL, NULL );
        if( retval < 0 )
            throw system_error( "Error occurred while in the main select:" );

        // must actually never happen
        if( 0 == retval )
            throw system_error( "The main select has timeout." );

        if( FD_ISSET( fd_first, &readset ) )
        {
            _node->dealWithData( CNode::nodeSocketFirst );
        }
        if( FD_ISSET( fd_second, &readset ) )
        {
            _node->dealWithData( CNode::nodeSocketSecond );
        }


        // we got a signal for update
        // reading everything from the pipe and exiting from the function
        if( FD_ISSET( m_fdSignalPipe, &readset ) )
        {
            const int read_size = 20;
            char buf[read_size];
            int numread( 0 );
            //  do
            //  {
            numread = read( m_fdSignalPipe, buf, read_size );
            //  }
            //   while ( numread > 0 );

            throw system_error( "Got a wake up signal from the signal pipe." );
        }

    }
}
//=============================================================================
void CAgentClient::createPROOFCfg()
{
    DebugLog( erOK, "Creating a PROOF configuration file..." );

    string proofCfg( getPROOFCfg() );
    ofstream f( proofCfg.c_str() );
    if( !f.is_open() )
        throw runtime_error( "can't open " + proofCfg + " for writing." );

    // getting local host name
    string host;
    MiscCommon::get_hostname( &host );
    f << "worker " << host << " perf=100" << std::endl;
}
