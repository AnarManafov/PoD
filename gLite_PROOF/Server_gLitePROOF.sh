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

start() 
{
    echo "Starting..."
    xrootd -c xpd.cf -b -l /tmp/xpd.log stop
    
    ./proofagent -d -i server -p /tmp/ -c proofagent.cfg.xml --start
    
    return 0
}

stop()
{
    echo "Stoping..."
    pkill -9 xrootd
   #pkill -9 proofserv

    ./proofagent -d -i server -p /tmp/ --stop
    
    return 0
}

status()
{
    echo `ps -A | grep xrootd`
    ./proofagent -d -i server -p /tmp/ --status
}

case "$1" in
    start)	
	start
	RETVAL=$?
	;;
    stop)
	stop
	RETVAL=$?
	;;
    status)
	status
	;;
esac

exit $RETVAL
