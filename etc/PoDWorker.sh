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
#        Copyright (c) 2007-2011 GSI, Scientific Computing devision. All rights reserved.
#*************************************************************************/
#
# Arguments:
# $1 - a number of PROOF workers to spawn (default is 1). Used by SSH plug-in.
#
# Environment needed by workers:
# A job script (like Job.pbs, Job.lsf, etc.) is responsible to export the following variables:
# $POD_SHARED_HOME (optional) 	 : defined if a shared home file system is detected (where PoD is installed)
# $POD_UI_LOG_LOCATION 		 : log dir on the PoD UI
# $POD_UI_LOCATION     		 : $POD_LOCATION of the PoD UI
#
#
# Notes:
#
# 1. The script redefines $POD_LOCATION to WN's working dir.
# 2. If $POD_SHARED_HOME is defined, then the script will try to use binaries from the Server directly.
#

# current working dir
WD=$(pwd)
#
eval LOCK_FILE="$WD/PoDWorker.lock"
eval PID_FILE="$WD/PoDWorker.pid"
eval POD_CFG="$WD/PoD.cfg"
eval USER_SCRIPT="$WD/user_worker_env.sh"
eval XPD_CFG="$WD/xpd.cf"
# bin name:
# <pakage>-<version>-<OS>-<ARCH>.tar.gz
BASE_NAME="pod-wrk-bin"
BIN_REPO="http://pod.gsi.de/releases/add/"

#=============================================================================
# ***** LOG function *****
#=============================================================================
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
# ***** wait_and_kill *****
# ************************************************************************
wait_and_kill()
{
   # if after 10 sec. a given process still exists send a non-ignorable kill
   WAIT_TIME=10
   cnt=0
   while $(kill -0 $1 &>/dev/null); do
      cnt=$(expr $cnt + 1)
      if [ $cnt -gt $WAIT_TIME ]; then
         logMsg "Process \"$1\" doesn't want to stop. Forcing a non-ignorable kill..."
         kill -9 $1
         break
      fi
      sleep 1
   done
}
# ************************************************************************
# ***** detects pid and port of XPROOFD *****
# ************************************************************************
xproofd_info()
{
   # Every time new xproofd is started it creates <adminpath>/.xproofd.port directory.
   # In this directory an xrd pid file is located.
   # There is one problem with the file, is that even when xrootd/xproofd is off already,
   # the file will be there in anyway. This complicates the algorithm of detecting of xproofd.

   # find xproofd pid file
   if [ ! -r "$XPD_CFG" ]; then
      return 1
   fi
   adminpath=$(cat "$XPD_CFG" | grep "worker.adminpath" | awk '{print $3}')
   if [ ! -d "$adminpath" ]; then
      return 1
   fi
   xpd_port=$(ls -a "$adminpath" | grep xproofd | awk -F. '{print $3}')
   xpd_pid=$(cat "$adminpath/.xproofd.$xpd_port/xrootd.pid" 2>/dev/null)
   if [ -n "$xpd_pid" ]; then
      kill -0 $xpd_pid &>/dev/null
      if (( $? != 0 )) ; then
         xpd_pid=""
      fi
   fi
}
#=============================================================================
# ***** clean_up *****
#=============================================================================
# ***** Perform program exit housekeeping *****
clean_up()
{
    logMsg "Starting the cleaning procedure..."

    # shut down PROOF
    xproofd_info
    if [ -n "$xpd_pid" ]; then
       # kill all proofserv, which are children of our xproofd.
       # PROOF sometime can't properly clean them and it could give us
       # some problem next time we start PROOF session
       for i in `ps -ef| awk '$3 == '$xpd_pid' { print $2 }'`
       do
         logMsg "killing proofserv: $i"
         kill -9 $i &>/dev/null
       done
    fi

    # Try to stop the server by first sending a TERM signal
    # And if after 10 sec. it still exists send a non-ignorable kill
    logMsg "Gracefully shut down PoD worker process(es): $xpd_pid $PODAGENT_PID"
    kill $PODAGENT_PID $xpd_pid &>/dev/null

    wait_and_kill $PODAGENT_PID
    wait_and_kill $xpd_pid
    
    # archive and remove the local proof directory
    proof_dir="$WD/proof"
   
    if [ -e "$proof_dir" ]; then
       # making an archive of proof logs
       # it will be transfered to a user
       tar -czPf proof_log.tgz $proof_dir
       logMsg "$proof_dir exists and will be deleted..."
       rm -rf $proof_dir
    fi

    # delete the content of the worker package
    # this is needed in case of condor RMS, for example
    # otherwise condor will transfer all files back
    # TODO: delete the rest of files as well
 # BUG: We must not delete pod-user-defaults and its dependencies,
 #      since many plug-ins use it in stage-out processes.
 #      Need to think how to delete it only in the condor plug-in case 
 #   to_delete=$(tar -ztf pod-worker)
 #   if (( $? == 0 )) ; then
 #      rm -vf $to_delete
 #   fi

    # remove lock file
    rm -f $LOCK_FILE
    rm -f $PID_FILE

    logMsg "done cleaning up."
    exit $1
}
#=============================================================================
# ***** xpd_detect *****
#=============================================================================
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
#=============================================================================
# ***** get_freeport *****
#=============================================================================
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
#=============================================================================
# ***** check_arch *****
# so far we support only Linux (amd64 and x86)
#=============================================================================
check_arch()
{
   OS=$(uname -s 2>&1)
   case "$OS" in
      "Linux"|"Darwin")
         ;;
      *)
         logMsg "Error: PoD doesn't support this operating system. Exiting..."
         clean_up 1
         ;;
   esac

   wn_host_arch=$(uname -m  2>&1)
   case "$wn_host_arch" in
      i[3-9]86*|x86|x86pc|k5|k6|k6_2|k6_3|k6-2|k6-3|pentium*|athlon*|i586_i686|i586-i686)
         host_arch=x86
         ;;
      x86_64)
         host_arch=amd64
         ;;
      *)
         logMsg "Error: unsupported architecture: $host_arch"
         clean_up 1
	 ;;
   esac

   logMsg "host's CPU/instruction set: $host_arch"
}
#=============================================================================
# ***** get_default_ROOT *****
# arg: $1 - is the architecture of the worker
#=============================================================================
get_default_ROOT()
{
   # define default ROOT packages
   # will be used on WNs if a user doesn't provide own ROOT
   if [ "$OS" == "Linux" ]; then
      case "$1" in
         x86)
            ROOT_ARC="root_v5.26.00.Linux-slc5-gcc4.3.tar.gz" ;;
         amd64)
            ROOT_ARC="root_v5.26.00.Linux-slc5_amd64-gcc4.3.tar.gz" ;;
      esac
   elif  [ "$OS" == "Darwin" ]; then
      case "$1" in
         x86)
            ROOT_ARC="" ;;
         amd64)
            ROOT_ARC="root_v5.28.00c.macosx106-x86_64-gcc-4.2.tar.gz" ;;
      esac   
   fi

   local set_my_rootsys=$($user_defaults -c $POD_CFG --key worker.set_my_rootsys)
   if (( $set_my_rootsys == 0 )); then
      logMsg "User requested to use a PoD default ROOT version. Downloading..."
      wget --no-verbose --tries=2 $BIN_REPO$ROOT_ARC || clean_up 1
      tar -xzf $ROOT_ARC || clean_up 1

      export ROOTSYS="$WD/root"
   else
      eval ROOTSYS_FROM_CFG=$($user_defaults -c $POD_CFG --key worker.my_rootsys)
      export ROOTSYS=$ROOTSYS_FROM_CFG
   fi

   if [ -z $ROOTSYS ]; then
      logMsg "Warning: ROOTSYS is not defined."
   else
      logMsg "using ROOTSYS: $ROOTSYS"
   fi
   
   export PATH=$ROOTSYS/bin:$PATH
   export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH 
   export LD_LIBRARY_PATH=$ROOTSYS/lib/root:$LD_LIBRARY_PATH

   # check binary
   xproofd -h > /dev/null 2>&1
   if (( $? != 0 )) ; then
      logMsg "Error: can't execute xproofd. Check your ROOT installation."
      clean_up 1
   fi
}
# ************************************************************************
#
# 				M A I N
#
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

# print the environment
env

logMsg "+++ PoD Worker START +++"
logMsg "Current working directory: $WD"

# extract PoD worker package
logMsg "Content of the worker package:"
tar -xzvf pod-worker

#Exporting PoD location
export POD_LOCATION=$WD

# getting the version of PoD
PKG_VERSION=$(cat $WD/version)

# execute user's script if present
if [ -r $USER_SCRIPT ]; then
   logMsg "Sourcing a user defined environment script..."
   source $USER_SCRIPT
fi

# host's CPU/instruction set
check_arch

# **********************************************************************
# ***** try to use pre-compiled bins from PoD Server *****
logMsg "PoD worker runs on $OS-$wn_host_arch"
# check first whether we can use binaries from the PoD server directly.
# Using these bins is more preferable, than using generic bins from the worker package.
need_bin_pkgs="TRUE"
if [ -n "$POD_SHARED_HOME" ]; then
   logMsg "A shared home file system is detected."
   logMsg "Let's try to use PoD binaries directly from the server."
   
   # check binary
   bin_to_test="$POD_UI_LOCATION/bin/pod-agent"
   $bin_to_test --version > /dev/null 2>&1
   if (( $? == 0 )) ; then
      logMsg "Server's bins are working."
      need_bin_pkgs=""
      cp "$POD_UI_LOCATION/bin/pod-user-defaults" $WD/
      cp "$POD_UI_LOCATION/bin/pod-agent" $WD
   else
      logMsg "Can't use server's bins. Will try with the worker package."    
   fi
fi

# use pre-compiled binaries from the worker package
if [ -n need_bin_pkgs ]; then
   # ***** prepare pre-compiled wn binaries *****
   WN_BIN_ARC="$WD/$BASE_NAME-$PKG_VERSION-$OS-$host_arch.tar.gz"
   if [ ! -f "$WN_BIN_ARC" ]; then
      logMsg "Error: Can't find WN pre-compiled bin.: $WN_BIN_ARC"
      clean_up 1
   fi
   # un-tar without creating a sub-directory
   tar --strip-components=1 -xzf $WN_BIN_ARC || clean_up 1

   export PATH=$WD:$PATH 
   export LD_LIBRARY_PATH=$WD:$LD_LIBRARY_PATH

   # Transmitting an executable through the InputSandbox does not preserve execute permissions
   chmod +x $WD/pod-agent
   chmod +x $WD/pod-user-defaults
fi

user_defaults="$WD/pod-user-defaults"
pod_agent="$WD/pod-agent"

# check binary
$pod_agent --version > /dev/null 2>&1
if (( $? != 0 )) ; then
   logMsg "Error: Can't find a suitable pre-compiled binary for this system."
   clean_up 1
fi

UD_TEST=$($user_defaults -c $POD_CFG -V --key worker.work_dir)
if (( $? != 0 )) ; then
   logMsg "ERROR: Your PoD user defaults on the server can't be used with the version of PoD on workers."
   logMsg "If you want to continue to use this version of PoD, you have the following options:"
   logMsg "1. Fix your configuration file."
   logMsg "2. Recreate the file using: pod-user-defaults -f -d -c \"$(pod-user-defaults -p)\""
   clean_up 1
fi


# Use a default ROOT distr. if needed
get_default_ROOT $host_arch

# **********************************************************************
# export the location of the proof.conf file
eval POD_PROOFCFG_FILE="$WD/proof.conf"
export POD_PROOFCFG_FILE

# Using eval to force variable substitution
# changing _G_WRK_DIR to a working directory in the following files:
eval sed -i.bup 's%_G_WRK_DIR%$WD%g' $XPD_CFG
# populating the tmp dir.
_TMP_DIR=$(mktemp -d /tmp/PoDWorker_XXXXXXXXXX)
chmod 777 $_TMP_DIR
eval sed -i.bup 's%_G_WORKER_TMP_DIR%$_TMP_DIR%g' $XPD_CFG

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
  if (( $? != 0 )); then
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
if (( $? != 0 )); then
   logMsg "checking XPROOFD process: is NOT running"
   clean_up 1
else
   logMsg "checking XPROOFD process: running..."
fi

logMsg "starting pod-agent..."
# start pod-agent
if [ -n "$1" ]; then
   $pod_agent -c $POD_CFG -m worker --serverinfo $WD/server_info.cfg --workers $1 &
else
   $pod_agent -c $POD_CFG -m worker --serverinfo $WD/server_info.cfg &
fi
# wait for pod-agent's process
PODAGENT_PID=$!
wait $PODAGENT_PID

logMsg "--- DONE ---"

# Exit
clean_up 0

