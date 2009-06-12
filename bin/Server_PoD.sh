#! /bin/bash

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
# TODO: take this info from proofagent.cfg file
XRD_PORTS_RANGE_MIN=20000
XRD_PORTS_RANGE_MAX=21000
XPROOF_PORTS_RANGE_MIN=21001
XPROOF_PORTS_RANGE_MAX=22000
PROOFAGENT_PORTS_RANGE_MIN=22001
PROOFAGENT_PORTS_RANGE_MAX=23000
#######
# a number of seconds we wait until xrd is started 
XRD_START_TIMEOUT=3 

# ************************************************************************
# ***** detects ports for XRD and XPROOF  *****
xrd_detect()
{
# get a pid of our xrd. We get any xrd running by $UID
    XRD_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep -v grep | grep xrootd | awk '{print $1}'`
    
    if [ -n "$XRD_PID" ]; then
	echo "XRD is running under PID: "$XRD_PID
    else
	echo "XRD is NOT running"
	return 1
    fi
    
# getting an array of XRD LISTEN ports
# oreder: the lowerst port goes firstand its a XRD port.
# XPROOF port must be greater
    XRD_PORTS=(`lsof -P -w -a -c xrootd -u $UID -i -n |  grep LISTEN  | sed -n -e 's/.*:\([0-9]*\).(LISTEN)/\1/p' | sort -b -n -u`)
    
    echo "- XRD port: "${XRD_PORTS[0]}
    echo "- XPROOF port: "${XRD_PORTS[1]}
    return 0
}
# ************************************************************************
# ***** detects ports for PROOFAgent  *****
proofagent_detect()
{
# get a pid of our proofagent. We get any xrd running by $UID
    PA_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep -v grep | grep proofagent | awk '{print $1}'`
    
    if [ -n "$PA_PID" ]; then
	echo "PROOFAgent is running under PID: "$PA_PID
    else
	echo "PROOFAgent is NOT running"
	return 1
    fi
    
# getting an array of PA LISTEN ports
    PA_PORTS=(`lsof -P -w -a -c proofagent -u $UID -i -n |  grep LISTEN  | sed -n -e 's/.*:\([0-9]*\).(LISTEN)/\1/p' | sort -b -n -u`)
    
    echo "- PROOFAgent server port: "${PA_PORTS[0]}
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
# ***** START  *****
start() 
{
    # S T O P I N G the server first
    stop $1

    # S T A R T I N G the server
    echo "Starting PoD server..."
    # proof.conf must be presented before xrootd is started
    touch $POD_LOCATION/proof.conf

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

    # replacing ports in the PROOF example script
    regexp_xproof_port="s/\(TProof::Open([[:space:]]\"\).*:[0-9]*\(\"[[:space:]])\)/\1$(hostname -f):$NEW_XPROOF_PORT\2/g"
    regexp_xrd_port="s/\(root:\/\/\).*:[0-9]*/\1$(hostname -f):$NEW_XRD_PORT/g"
    sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" $POD_LOCATION/test/simple_test0.C > $POD_LOCATION/test/simple_test0.C.temp
    mv $POD_LOCATION/test/simple_test0.C.temp $POD_LOCATION/test/simple_test0.C

    # Start XRD
    ####
    xrootd -n PoDServer -c $POD_LOCATION/etc/xpd.cf -b -l $POD_LOCATION/log/xpd.log
    
    sleep $XRD_START_TIMEOUT # let XRD to start
	
    # setting a port to listen for PROOFAgent server and server's host name
    regexp_listen="s/\(listen_port=\)[0-9]*/\1$NEW_PROOFAGENT_PORT/g"
    regexp_server="s/\(server_port=\)[0-9]*/\1$NEW_PROOFAGENT_PORT/g"
    regexp_serverhostname="s/\(server_addr=\).*/\1$(hostname -f)/g"
    sed -e "$regexp_listen" $POD_LOCATION/etc/proofagent.cfg > $POD_LOCATION/etc/proofagent.cfg.temp
    mv $POD_LOCATION/etc/proofagent.cfg.temp $POD_LOCATION/etc/proofagent.cfg

    # PROOFAgent client configuration file
    sed -e "$regexp_server" -e "$regexp_serverhostname" $POD_LOCATION/etc/proofagent.client.cfg > $POD_LOCATION/etc/proofagent.client.cfg.temp
    mv $POD_LOCATION/etc/proofagent.client.cfg.temp $POD_LOCATION/etc/proofagent.client.cfg

    # Start Proofagent
    ####
    $POD_LOCATION/bin/proofagent -d -p "$1/" -c $POD_LOCATION/etc/proofagent.cfg --start
    
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

    $POD_LOCATION/bin/proofagent -d -p "$1/" -c $POD_LOCATION/etc/proofagent.cfg --stop
    
    return 0
}
# ************************************************************************
# ***** STATUS  *****
status()
{
    # XRD
    xrd_detect

    # PROOFAgent
    proofagent_detect
   
    # check that ROOTSYS is set
    if [ -z $ROOTSYS ]; then
       echo ""
       echo 'WARNING: $ROOTSYS is not set.'
    fi
}

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
