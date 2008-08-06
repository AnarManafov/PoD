#! /bin/bash

#/************************************************************************/
#/**
# * @file Server_gLitePROOF.sh
# * @brief a script, which starts gLitePROOF's server side
# * @author Anar Manafov A.Manafov@gsi.de
# *//*
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2007-05-15
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2007 GSI GridTeam. All rights reserved.
#*************************************************************************/
#
# Usage: ./Server_gLitePROOF.sh <pid_dir> start|stop|status 
#

start() 
{
    echo "Starting..."
    # proof.conf must be presented before xrootd is started
    touch ~/proof.conf

    olbd -c $GLITE_PROOF_LOCATION/etc/xpd.cf -b -l "$1/xpd.log"

    xrootd -c $GLITE_PROOF_LOCATION/etc/xpd.cf -b -l "$1/xpd.log"
    
    $GLITE_PROOF_LOCATION/bin/proofagent --validate -d -i server -p "$1/" -c $GLITE_PROOF_LOCATION/etc/proofagent.cfg.xml --start
    
    return 0
}

stop()
{
    echo "Stoping..."
    pkill -9 olbd
    pkill -9 xrootd
    pkill -9 proofserv

    $GLITE_PROOF_LOCATION/bin/proofagent -d -i server -p "$1/" --stop
    
    return 0
}

status()
{
    echo `ps -A | grep xrootd`
    echo `ps -A | grep olbd`
    $GLITE_PROOF_LOCATION/bin/proofagent -d -i server -p "$1/" --status
}

# checking the number of parameters
if [ $# -ne 2 ]; then
    echo "Usage: ./Server_gLitePROOF.sh <pid_dir> start|stop|status"
    exit 1
fi

# pid_dir must be a valid dir
if [ ! -e "$1" ]; then
    echo "error: pid director: \"$1\" doesn't exist!"
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
