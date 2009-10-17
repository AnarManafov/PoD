#!/usr/bin/env bash
#/************************************************************************/
#/**
# * @file Server_Pod.sh
# * @brief a script, which starts PoD's server side
# * @author Anar Manafov A.Manafov@gsi.de
# *//*
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2007-05-15
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
#*************************************************************************/
#
# Usage:
#      Server_PoD.sh <work_dir> start|stop|status 
#

#######
XRD_PORTS_RANGE_MIN=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.xrd_ports_range_min`
XRD_PORTS_RANGE_MAX=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.xrd_ports_range_max`
XPROOF_PORTS_RANGE_MIN=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.xproof_ports_range_min`
XPROOF_PORTS_RANGE_MAX=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.xproof_ports_range_max`
PROOFAGENT_PORTS_RANGE_MIN=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.agent_ports_range_min`
PROOFAGENT_PORTS_RANGE_MAX=`pod-user-defaults -c $POD_LOCATION/etc/PoD.cfg --key server.agent_ports_range_max`
#######
# a number of seconds we wait until xrd is started 
XRD_START_TIMEOUT=3 

server_status()
{
# get a pid of our xrd. We get any xrd running by $UID
    XRD_PID=(`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | egrep "xrootd|pod-agent" | grep -v grep | awk '{print $1}'`)
    
    if [ -n "$XRD_PID" ]; then
	echo "XROOTD is running under PID: "${XRD_PID[0]} ${XRD_PID[1]}
    else
	echo "XROOTD is NOT running"
	return 1
    fi
    
# getting an array of XRD LISTEN ports
# oreder: the lowerst port goes firstand its a XRD port.
# XPROOF port must be greater
    NETSTATS_RET=`netstat -n --inet --program --listening -t 2> /dev/null`
    XRD_PORTS=(`echo $NETSTATS_RET | grep "10077/"  |  awk '{print $4}' | sed 's/^.*://g' | sort -b -n -u`)
    echo "- XROOTD port: "${XRD_PORTS[0]}
    echo "- XPROOF port: "${XRD_PORTS[1]}

    AGENT_PORT=`echo $NETSTATS_RET | grep "/pod-agent"  |  awk '{print $4}' | sed 's/^.*://g' | sort -b -n -u`
    echo "- AGENT port: "$AGENT_PORT
    return 0
}

# ************************************************************************
# ***** detects ports for XRD and XPROOF  *****
xrd_detect()
{
# get a pid of our xrd. We get any xrd running by $UID
    XRD_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep "xrootd" | grep -v grep | awk '{print $1}'`
    
    if [ -n "$XRD_PID" ]; then
	echo "XROOTD is running under PID: "$XRD_PID
    else
	echo "XROOTD is NOT running"
	return 1
    fi
    
# getting an array of XRD LISTEN ports
# oreder: the lowerst port goes firstand its a XRD port.
# XPROOF port must be greater
    XRD_PORTS=(`netstat -n --inet --program --listening -t 2> /dev/null | grep $XRD_PID  |  awk '{print $4}' | sed 's/^.*://g' | sort -b -n -u`)
    echo "- XROOTD port: "${XRD_PORTS[0]}
    echo "- XPROOF port: "${XRD_PORTS[1]}
    return 0
}
# ************************************************************************
# ***** detects ports for pod-agent  *****
pod_agent_detect()
{
# get a pid of our pod-agent. We get any pod-agent running by $UID
    PA_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' |  grep pod-agent | grep -v grep | awk '{print $1}'`
    
    if [ -n "$PA_PID" ]; then
	echo "PoD Agent is running under PID: "$PA_PID
    else
	echo "PoD Agent is NOT running"
	return 1
    fi
    
# getting an array of pod-agent LISTEN ports
    PA_PORTS=(`netstat -n --inet --program --listening -t 2> /dev/null | grep "$PA_PID/" |  awk '{print $4}' | sed 's/^.*://g' | sort -b -n -u`)
    
    echo "- PoD Agent server port: "${PA_PORTS[0]}
    return 0
}
# ************************************************************************
# ***** returns a free port from a given range  *****
get_freeport()
{
    perl -e '
    use IO::Socket;
    my $port = $ARGV[0];
    for (; $port < $ARGV[1]; $port++) {
        $fh = IO::Socket::INET->new( Proto     => "tcp",
                                     LocalPort => $port,
                                     Listen    => SOMAXCONN,
                                     Reuse     => 0);
        if ($fh){
            $fh->close();
            print "$port";
            exit(0);
        }


    }
    die "%Error: Cant find free socket port\n";
    exit(1);
' $1 $2
}
# ************************************************************************
# ***** returns a free port from a given range  *****
# takes two parameters, pod agent server host name and port number
create_agent_server_info_file()
{
    SERVERINFO_FILE="$POD_LOCATION/etc/server_info.cfg"
    echo "[server]" > $SERVERINFO_FILE
    echo "host=$1" >> $SERVERINFO_FILE
    echo "port=$2" >> $SERVERINFO_FILE
}
# ************************************************************************
# ***** START  *****
start() 
{
    # S T O P I N G the server first
    stop $1

    # S T A R T I N G the server
    echo "Starting PoD server..."
    # proof.conf must be presented before xrootd is started
    touch $POD_PROOFCFG_FILE

    NEW_XRD_PORT=`get_freeport $XRD_PORTS_RANGE_MIN $XRD_PORTS_RANGE_MAX`
    NEW_XPROOF_PORT=`get_freeport $XPROOF_PORTS_RANGE_MIN $XPROOF_PORTS_RANGE_MAX`
    NEW_PROOFAGENT_PORT=`get_freeport $PROOFAGENT_PORTS_RANGE_MIN $PROOFAGENT_PORTS_RANGE_MAX`
    echo "using XRD port:"$NEW_XRD_PORT
    echo "using XPROOF port:"$NEW_XPROOF_PORT
    echo "using PROOFAgent server port: "$NEW_PROOFAGENT_PORT

    # updating XRD configuration file
    regexp_xrd_port="s/\(xrd.port[[:space:]]*\)[0-9]*/\1$NEW_XRD_PORT/g"
    regexp_xproof_port="s/\(xrd.protocol[[:space:]]xproofd:\)[0-9]*/\1$NEW_XPROOF_PORT/g"
    regexp_server_host="s/\(if[[:space:]]\).*\([[:space:]]#SERVERHOST DONT EDIT THIS LINE\)/\1$(hostname -f)\2/g"
    sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" -e "$regexp_server_host" $POD_LOCATION/etc/xpd.cf > $POD_LOCATION/etc/xpd.cf.temp
    mv $POD_LOCATION/etc/xpd.cf.temp $POD_LOCATION/etc/xpd.cf

    # generate a helper header
    # this helps to keep track of PROOF master's host and port for user's scripts
    HEADER_HELPER="$POD_LOCATION/etc/pod-master.h"
    echo "#ifndef _POD_MASTER_H_" > $HEADER_HELPER
    echo "#define _POD_MASTER_H_" >> $HEADER_HELPER
    echo "#define POD_MASTER_HOST \"$(hostname -f)\"" >> $HEADER_HELPER
    echo "#define POD_XPROOF_PORT \"$NEW_XPROOF_PORT\"" >> $HEADER_HELPER
    echo "#define POD_XROOTD_PORT \"$NEW_XRD_PORT\"" >> $HEADER_HELPER
    echo "#endif" >> $HEADER_HELPER

    # Start XRD
    ####
    xrootd -n PoDServer -c $POD_LOCATION/etc/xpd.cf -b -l $POD_LOCATION/log/xpd.log
    
    sleep $XRD_START_TIMEOUT # let XRD to start
	
    # setting a port to listen for pod-agent server and server's host name
    ####
    create_agent_server_info_file $(hostname -f) $NEW_PROOFAGENT_PORT

    # Start Proofagent
    ####
    $POD_LOCATION/bin/pod-agent -d -m server -p "$1/" -c $POD_LOCATION/etc/PoD.cfg --start
    
    return 0
}
# ************************************************************************
# ***** STOP  *****
stop()
{
    echo "Stoping PoD server..."

    #TODO: make it less aggressive
    pkill -9 -U $UID xrootd
    pkill -9 -U $UID proofserv

    $POD_LOCATION/bin/pod-agent -d -p "$1/" -c $POD_LOCATION/etc/PoD.cfg --stop
    
    return 0
}
# ************************************************************************
# ***** STATUS  *****
status()
{
server_status
    # XRD
   # xrd_detect

    # PROOFAgent
    #pod_agent_detect
   
    # check that ROOTSYS is set
    if [ -z $ROOTSYS ]; then
       echo ""
       echo 'WARNING: $ROOTSYS is not set.'
    fi
}



# ************************************************************************
# ***** MAIN  *****
# checking the number of parameters
if [ $# -ne 2 ]; then
    echo "Usage: ./Server_PoD.sh <working dir> start|stop|status"
    exit 1
fi

# work_dir must be a valid dir
if [ ! -e "$1" ]; then
    echo "error: working director: \"$1\" doesn't exist!"
    exit 1
fi

# star|stop|status
case "$2" in
    start)	
	start $1
	RETVAL=$?
	;;
    stop)
	stop $1
	RETVAL=$?
	;;
    status)
	status $1
	;;
esac

exit $RETVAL
