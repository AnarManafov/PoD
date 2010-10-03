#!/usr/bin/env bash
#/************************************************************************/
#/**
# * @file PoDWorker.sh
# * @brief PoDWorker - is a job script
# * @author Anar Manafov A.Manafov@gsi.de
# *//*
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2007-05-15
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2007-2010 GSI, Scientific Computing devision. All rights reserved.
#*************************************************************************/
#
# Arguments:
# $1 - a number of PROOF workers to spawn (default is 1). Used by SSH plug-in.
#
#
# current working dir
WD=$(pwd)
#
LOCK_FILE="$WD/PoDWorker.lock"
PID_FILE="$WD/PoDWorker.pid"
POD_CFG="$WD/PoD.cfg"
USER_SCRIPT="$WD/user.worker_env.sh"
#
# ************************************************************************
# F U N C T I O N S
# ************************************************************************
# ***** Log function  *****
logMsg()
{
# date format
#RFC-2822:
# *    www, dd mmm yyyy hh:mm:ss +zzzz
#
# Don't use date -R since it's a GNU specific implementation (doesn't work on Mac, for example)
    echo -e "***\t[$(date '+%a, %d %b %Y %T %z')]\t$1"
}
# ************************************************************************
# ***** Perform program exit housekeeping *****
clean_up()
{
    logMsg "Starting the cleaning procedure..."
    # Try to kill pod-agent by first sending a TERM signal
    # And if after 10 sec. it still exists send a non-ignorable kill
    WAIT_TIME=10
    kill $PODAGENT_PID
    cnt=0
    while $(kill -0 $PODAGENT_PID &>/dev/null); do
       cnt=$(expr $cnt + 1)
       if [ $cnt -gt $WAIT_TIME ]; then
          kill -9 $PODAGENT_PID
          break
       fi
       sleep 1
    done

    # force kill of xproofd and proof processes
    # pod-agent will shutdown automatically if there will be no xproofd 
    pkill -9 -U $UID xproofd
    pkill -9 -U $UID proofserv
    
    # archive and remove the local proof directory
    proof_dir="$WD/proof"
   
    if [ -e "$proof_dir" ]; then
	# making an archive of proof logs
	# it will be transfered to a user
	tar -czvf proof_log.tgz $proof_dir
	logMsg "$proof_dir exists and will be deleted..."
	rm -rf $proof_dir
    fi

    # remove lock file
    rm -f $LOCK_FILE
    rm -f $PID_FILE

    logMsg "done cleaning up."
    exit $1
}
# ************************************************************************
# ***** detects ports of XPROOFD  *****
# return 1 if the XPD port were not detected, otherwise returns 0
# sets XPD_PID to a pid of a found XPD
# sets XPROOF_PORT - XPD port
xpd_detect()
{
    # get a pid of our xpd. We get any xpd running by $UID
    XPD_PID=$(ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep xproofd | grep -v grep | awk '{print $1}')
    
    if [ -n "$XPD_PID" ]; then
	logMsg "XPROOFD is running under PID: "$XPD_PID
    else
	logMsg "XPROOFD is NOT running"
	return 0
    fi
    
    var0=0
    RETRY_CNT=15
    # we try for 15 times to detect xpd ports
    # it is needed in case when several PoD workers are started in the same time on one machine
    while [ "$var0" -lt "$RETRY_CNT" ]
      do
      logMsg "detecting xproofd ports. Try $var0"
      # getting an array of XPD LISTEN ports
      # change a string separator
      O=$IFS IFS=$'\n' NETSTAT_RET=($(netstat -n --program --listening -t 2>/dev/null | grep "xproofd")) IFS=$O;
      
      # look for ports of the server
      for(( i = 0; i < ${#NETSTAT_RET[@]}; ++i ))
	do
	port=$(echo ${NETSTAT_RET[$i]} | awk '{print $4}' | sed 's/^.*://g')
	if [ -n "$port" ]; then
	    if (( ($port >= $XPROOF_PORTS_RANGE_MIN) && ($port <= $XPROOF_PORTS_RANGE_MAX) )); then
		XPROOF_PORT=$port
                break
	    fi
	fi
      done
       
      logMsg "PoD has detected XPROOFD port: "$XPROOF_PORT
      if [ -n "$XPROOF_PORT" ]; then
	  return 0
      else
	  var0=`expr $var0 + 1`
	  # TODO: move the magic number to vars
	  sleep 5
      fi
    done

    return 1
}
# ************************************************************************
# ***** returns a free port from a given range  *****
get_freeport()
{
    for(( i = $1; i < $2; ++i ))
    do
       netstat -ant 2>/dev/null | grep ":$i" | egrep -i "listen|time_wait" &>/dev/null || { echo $i; exit 0; }
    done

    echo "Error: Cant find free socket port"
    exit 1
}
# ************************************************************************

# ************************************************************************
# M A I N
# ************************************************************************

# check for lock file
if lockfile -! -r1 $LOCK_FILE 
then
  logMsg "Error: There is already a PoD session running in the directory: $WD"
  exit 1
fi
echo $$ > $PID_FILE

# handle signals
trap clean_up SIGHUP SIGINT SIGTERM 

logMsg "+++ PoD Worker START +++"
logMsg "Current working directory: $WD"

# extract PoD worker package
logMsg "Content of the worker package:"
tar -xzvf pod-worker.tar.gz

#Exporting PoD location
export POD_LOCATION=$WD

# execute user's script if present
if [ -r $USER_SCRIPT ]; then
   logMsg "Sourcing a user defined environment script..."
   source $USER_SCRIPT
fi

# host's CPU/instruction set
# so far we support only Linux (amd64 and x86)
OS=$(uname -s 2>&1)
if [ "$OS" != "Linux" ]; then
    logMsg "Error: PoD doesn't support this operating system. Exiting..."
    exit 1
fi

host_arch=$( uname -m  2>&1)
case "$host_arch" in
    i[3-9]86*|x86|x86pc|k5|k6|k6_2|k6_3|k6-2|k6-3|pentium*|athlon*|i586_i686|i586-i686)
        host_arch=x86
        ;;
    x86_64)
        host_arch=amd64
        ;;
    *)
        logMsg "Error: unsupported architecture: $host_arch"
	exit 1
	;;
esac

logMsg "host's CPU/instruction set: $host_arch"

case "$host_arch" in
    x86)
	ROOFAGENT_ARC="pod-agent-2.3-Linux-x86-gcc4.1.2.tar.gz"
	ROOT_ARC="root_v5.26.00.Linux-slc5-gcc4.3.tar.gz" ;;
    amd64)
        PROOFAGENT_ARC="pod-agent-2.3-Linux-amd64-gcc4.1.2.tar.gz"
        ROOT_ARC="root_v5.26.00.Linux-slc5_amd64-gcc4.3.tar.gz" ;;
esac

RELEASE_REPO="http://pod.gsi.de/releases/add/"

# **********************************************************************
# ***** getting pod-agent from the repository site *****
wget --no-verbose --tries=2 $RELEASE_REPO$PROOFAGENT_ARC  || clean_up 1
tar -xzf $PROOFAGENT_ARC || clean_up 1

export PROOFAGENTSYS="$WD/pod-agent"
export PATH=$PROOFAGENTSYS:$PATH 
export LD_LIBRARY_PATH=$PROOFAGENTSYS:$LD_LIBRARY_PATH
user_defaults="$PROOFAGENTSYS/pod-user-defaults"


# Transmitting an executable through the InputSandbox does not preserve execute permissions
if [ ! -x $PROOFAGENTSYS/pod-agent ]; then 
    chmod +x $PROOFAGENTSYS/pod-agent
fi
if [ ! -x $PROOFAGENTSYS/pod-user-defaults ]; then 
    chmod +x $PROOFAGENTSYS/pod-user-defaults
fi

# ****************
# ***** ROOT *****
set_my_rootsys=$($user_defaults -c $POD_CFG --key worker.set_my_rootsys)
if [ "$set_my_rootsys" = "no" ]; then
    wget --no-verbose --tries=2 $RELEASE_REPO$ROOT_ARC || clean_up 1
    tar -xzf $ROOT_ARC || clean_up 1

    export ROOTSYS="$WD/root"
else
    eval ROOTSYS_FROM_CFG=$($user_defaults -c $POD_CFG --key worker.my_rootsys)
    export ROOTSYS=$ROOTSYS_FROM_CFG
fi
logMsg "using ROOTSYS: $ROOTSYS"
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH 
export LD_LIBRARY_PATH=$ROOTSYS/lib/root:$LD_LIBRARY_PATH 

# **********************************************************************
# export the location of the proof.cfg file
eval POD_PROOFCFG_FILE=$($user_defaults -c $POD_CFG --key worker.proof_cfg_path)
export POD_PROOFCFG_FILE

# Using eval to force variable substitution
# changing _G_WRK_DIR to a working directory in the following files:
eval sed -i 's%_G_WRK_DIR%$WD%g' ./xpd.cf
# populating the tmp dir.
_TMP_DIR=$(mktemp -d /tmp/PoDWorker_XXXXXXXXXX)
chmod 777 $_TMP_DIR
eval sed -i 's%_G_WORKER_TMP_DIR%$_TMP_DIR%g' ./xpd.cf


# creating an empty proof.conf, so that xproof will be happy
touch $POD_PROOFCFG_FILE

# user defaults for ports
XPROOF_PORTS_RANGE_MIN=$($user_defaults -c $POD_CFG --key worker.xproof_ports_range_min)
XPROOF_PORTS_RANGE_MAX=$($user_defaults -c $POD_CFG --key worker.xproof_ports_range_max)

# we try for 5 times to detect/start xpd
# it is needed in case when several PoD workers are started in the same time on one machine
COUNT=0
MAX_COUNT=5
while [ "$COUNT" -lt "$MAX_COUNT" ]
  do
  # detecting whether xpd is running and on which port is listening
  xpd_detect
  return_val=$?
  if [ "X$return_val" != "X0" ]; then
      logMsg "problem to detect XPD/XPD ports. Exiting..."
      clean_up 1
  fi
  
  if [ -n "$XPD_PID" ]; then
      # use existing ports for xpd
      logMsg "found a running xproofd instance with pid: "$XPD_PID
      POD_XPROOF_PORT_TOSET=$XPROOF_PORT
  else
      # if xproofd is not yet running on this machine for this user, try to start it
      logMsg "xproofd is not running yet on this machine for this user."
      POD_XPROOF_PORT_TOSET=$(get_freeport $XPROOF_PORTS_RANGE_MIN $XPROOF_PORTS_RANGE_MAX)
  fi
  logMsg "using XPROOF port: "$POD_XPROOF_PORT_TOSET
  
  # updating XPD configuration file. Needed even if another scrip has already started an xproofd process,
  # since we might want to use port's info somewhere else.
  regexp_xpd_port="s/\(xpd.port[[:space:]]*\)[0-9]*/\1$POD_XPROOF_PORT_TOSET/g"
  sed -e "$regexp_xpd_port" -e "$regexp_xproof_port" $WD/xpd.cf > $WD/xpd.cf.temp
  mv $WD/xpd.cf.temp $WD/xpd.cf

  # break the loop if xproofd is running already
  if [ -n "$XPD_PID" ]; then
      break
  fi

  logMsg "starting xproofd..."
  xproofd -c $WD/xpd.cf -b -l $WD/xpd.log
  #give xproofd some time to start
  sleep 3

  # loop counter
  COUNT=$(expr $COUNT + 1)
done

# detect that xproofd failed to start
XPD=$(pgrep -U $UID xproofd)
XPD_RET_VAL=$?
if [ "X$XPD_RET_VAL" = "X0" ]; then
    logMsg "checking XPROOFD process: running..."
else
    logMsg "checking XPROOFD process: is NOT running"
    clean_up 1
fi

logMsg "starting pod-agent..."
# start pod-agent
if [ -n "$1" ]; then
	$PROOFAGENTSYS/pod-agent -c $POD_CFG -m worker --serverinfo $WD/server_info.cfg --proofport $POD_XPROOF_PORT_TOSET --workers $1 &
else
	$PROOFAGENTSYS/pod-agent -c $POD_CFG -m worker --serverinfo $WD/server_info.cfg --proofport $POD_XPROOF_PORT_TOSET &
fi
# wait for pod-agent's process
PODAGENT_PID=$!
wait $PODAGENT_PID

logMsg "--- DONE ---"

# Exit
clean_up 0

