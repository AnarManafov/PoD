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


# ************************************************************************
# ***** detects ports for XRD and XPROOF  *****
xrd_detect()
{
# get a pid of our xrd. We get any xrd running by $UID
    XRD_PID=`ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep -v grep | grep xrootd| awk '{print $1}'`
    
    if [ -n "$XRD_PID" ]
    then
	echo "XRD is running under PID: "$XRD_PID
    else
	echo "XRD is NOT running"
	return 1
    fi
    
# getting an array of XRD LISTEN ports
# oreder: the lowerst port goes firstand its a XRD port.
# XPROOF port must be greater
    XRD_PORTS=(`lsof -w -a -c xrootd -u $UID -i -n |  grep LISTEN  | sed -n -e 's/.*:\([0-9]*\).(LISTEN)/\1/p' | sort -b -n -u`)
    
    echo "-XRD port: "${XRD_PORTS[0]}
    echo "-XPROOF port: "${XRD_PORTS[1]}
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
    touch ~/proof.conf
    
    XRD_PORTS_RANGE_MIN=20000
    XRD_PORTS_RANGE_MAX=21000
    XPROOF_PORTS_RANGE_MIN=21001
    XPROOF_PORTS_RANGE_MAX=22000
    PROOFAGENT_PORTS_RANGE_MIN=22001
    PROOFAGENT_PORTS_RANGE_MAX=23000

    NEW_XRD_PORT=`get_freeport $XRD_PORTS_RANGE_MIN $XRD_PORTS_RANGE_MAX`
    NEW_XPROOF_PORT=`get_freeport $XPROOF_PORTS_RANGE_MIN $XPROOF_PORTS_RANGE_MAX`
    NEW_PROOFAGENT_PORT=`get_freeport $PROOFAGENT_PORTS_RANGE_MIN $PROOFAGENT_PORTS_RANGE_MAX`
    echo "using XRD port:"$NEW_XRD_PORT
    echo "using XPROOF port:"$NEW_XPROOF_PORT
    echo "using PROOFAgent server port: "$NEW_PROOFAGENT_PORT
    
    # updating XRD configuration file
    regexp_xrd_port="s/\(xrd.port[[:space:]]*\)[0-9]*/\1$NEW_XRD_PORT/g"
    regexp_xproof_port="s/\(xrd.protocol[[:space:]]xproofd:\)[0-9]*/\1$NEW_XPROOF_PORT/g"
    sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" $GLITE_PROOF_LOCATION/etc/xpd.cf > $GLITE_PROOF_LOCATION/etc/xpd.cf.temp
    mv $GLITE_PROOF_LOCATION/etc/xpd.cf.temp $GLITE_PROOF_LOCATION/etc/xpd.cf

    # replacing ports in the PROOF example script
    regexp_xproof_port="s/\(TProof::Open(\"\).*:[0-9]*\(\")\)/\1$(hostname -f):$NEW_XPROOF_PORT\2/g"
    regexp_xrd_port="s/\(root:\/\/\).*:[0-9]*/\1$(hostname -f):$NEW_XRD_PORT/g"
    sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" $GLITE_PROOF_LOCATION/test/simple_test0.C > $GLITE_PROOF_LOCATION/test/simple_test0.C.temp
    mv $GLITE_PROOF_LOCATION/test/simple_test0.C.temp $GLITE_PROOF_LOCATION/test/simple_test0.C

    # Start XRD
    #
    xrootd -n PoDServer -c $GLITE_PROOF_LOCATION/etc/xpd.cf -b -l "$1/xpd.log"
    
    # setting a port to listen for PROOFAgent server and server's host name
    regexp_listen="s/\(<listen_port>\)[0-9]*\(<\/listen_port>\)/\1$NEW_PROOFAGENT_PORT\2/g"
    regexp_server="s/\(<server_port>\)[0-9]*\(<\/server_port>\)/\1$NEW_PROOFAGENT_PORT\2/g"
    regexp_serverhostname="s/\(<server_addr>\).*\(<\/server_addr>\)/\1$(hostname -f)\2/g"
    sed -e "$regexp_listen" -e "$regexp_server" -e "$regexp_serverhostname" $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml > $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml.temp
    mv $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml.temp $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml

    # Start Proofagent
    #
    $GLITE_PROOF_LOCATION/bin/proofagent --validate -d -i server -p "$1/" -c $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml --start
    
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

    $GLITE_PROOF_LOCATION/bin/proofagent -d -i server -p "$1/" --stop
    
    return 0
}
# ************************************************************************
# ***** STATUS  *****
status()
{
    # XRD
    xrd_detect

    # PROOFAgent
    $GLITE_PROOF_LOCATION/bin/proofagent -d -i server -p "$1/" --status
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
