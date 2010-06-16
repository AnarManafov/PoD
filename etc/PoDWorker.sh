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
#        Copyright (c) 2007-2010 GSI GridTeam. All rights reserved.
#*************************************************************************/
#
# current working dir
WD=$(pwd)
#
LOCK_FILE="$WD/PoDWorker.lock"
PID_FILE="$WD/PoDWorker.pid"
POD_CFG="$WD/PoD.cfg"
#
# ************************************************************************
# F U N C T I O N S
# ************************************************************************
# ***** Log function  *****
logMsg()
{
    echo "*** [$(date -R)]   $1"
}
# ************************************************************************
# ***** Perform program exit housekeeping *****
clean_up()
{
    # force kill of xrootd and proof processes
    # pod-agent will shutdown automatically if there will be no xrootd 
    pkill -9 -U $UID xrootd
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
# ***** detects ports for XRD and XPROOF  *****
# return 1 if XRD/XPD ports were not detected, otherwise returns 0
# sets XRD_PID to a pid of a found XRD
# sets XRD_PORT - XRD port
# sets XPROOF_PORT - XPD port
xrd_detect()
{
    # get a pid of our xrd. We get any xrd running by $UID
    XRD_PID=$(ps -w -u$UID -o pid,args | awk '{print $1" "$2}' | grep xrootd | grep -v grep | awk '{print $1}')
    
    if [ -n "$XRD_PID" ]; then
	logMsg "XRD is running under PID: "$XRD_PID
    else
	logMsg "XRD is NOT running"
	return 0
    fi
    
    var0=0
    RETRY_CNT=10
    # we try for 10 times to detect xrd ports
    # it is needed in case when several PoD workers are started in the same time on one machine
    while [ "$var0" -lt "$RETRY_CNT" ]
      do
      logMsg "detecting xrd ports. Try $var0"
      # getting an array of XRD LISTEN ports
      # change a string separator
      O=$IFS IFS=$'\n' NETSTAT_RET=($(netstat -n --program --listening -t 2>/dev/null | grep "xrootd")) IFS=$O;
      
      # look for ports of the server
      for(( i = 0; i < ${#NETSTAT_RET[@]}; ++i ))
	do
	port=$(echo ${NETSTAT_RET[$i]} | awk '{print $4}' | sed 's/^.*://g')
	if [ -n "$port" ]; then
	    if (( ($port >= $XRD_PORTS_RANGE_MIN) && ($port <= $XRD_PORTS_RANGE_MAX) )); then
		XRD_PORT=$port
	    elif (( ($port >= $XPROOF_PORTS_RANGE_MIN) && ($port <= $XPROOF_PORTS_RANGE_MAX) )); then
		XPROOF_PORT=$port
	    fi
	fi
      done
       
      logMsg "PoD has detected XRD port: "$XRD_PORT
      logMsg "PoD has detected XPROOF port: "$XPROOF_PORT
      if [ -n "$XRD_PORT" ] && [ -n "$XPROOF_PORT" ]; then
	  return 0
      else
	  var0=`expr $var0 + 1`
	  # TODO: move the magic number to a var
	  sleep 5
      fi
    done

    return 1
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

logMsg "+++ START +++"
logMsg "Current working directory: $WD"

user_defaults="$WD/pod-user-defaults-lite"

# extract PoD worker package
tar -xzvf pod-worker.tar.gz

#Exporting PoD variables
export POD_LOCATION=$WD
eval POD_PROOFCFG_FILE=$($user_defaults -c $POD_CFG --section worker --key proof_cfg_path)
export POD_PROOFCFG_FILE

# Using eval to force variable substitution
# changing _G_WRK_DIR to a working directory in the following files:
eval sed -i 's%_G_WRK_DIR%$WD%g' ./xpd.cf
# populating the tmp dir.
_TMP_DIR=$(mktemp -d /tmp/PoDWorker_XXXXXXXXXX)
eval sed -i 's%_G_WORKER_TMP_DIR%$_TMP_DIR%g' ./xpd.cf

# host's CPU/instruction set
# so far we support only Linux (amd64 and x86)
OS=$(uname -s 2>&1)
if [ "$OS" != "Linux" ]; then
    logMsg "Error: PoD doen't support this operating system, exiting..."
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

logMsg "host's CPU/instruction set: " $host_arch

case "$host_arch" in
    x86)
	PROOFAGENT_ARC="pod-agent-2_1_4a-x86-linux-gcc_4_1.tar.gz"
	ROOT_ARC="root_v5.26.00.Linux-slc5-gcc4.3.tar.gz" ;;
    amd64)
        PROOFAGENT_ARC="pod-agent-2_1_4a-x86_64-linux-gcc_4_1.tar.gz"
        ROOT_ARC="root_v5.26.00.Linux-slc5_amd64-gcc4.3.tar.gz" ;;
esac

RELEASE_REPO="http://pod.gsi.de/releases/add/"
# ****************
# ***** ROOT *****
set_my_rootsys=$($user_defaults -c $POD_CFG --section worker --key set_my_rootsys)
if [ "$set_my_rootsys" = "no" ]; then
    wget --no-verbose --tries=2 $RELEASE_REPO$ROOT_ARC || clean_up 1
    tar -xzf $ROOT_ARC || clean_up 1

    export ROOTSYS="$WD/root"
    export PATH=$ROOTSYS/bin:$PATH
    export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
else
    eval ROOTSYS_FROM_CFG=$($user_defaults -c $POD_CFG --section worker --key my_rootsys)
    export ROOTSYS=$ROOTSYS_FROM_CFG
    export PATH=$ROOTSYS/bin:$PATH
    export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH 
fi


# **********************
# ***** getting pod-agent from the repository site *****
wget --no-verbose --tries=2 $RELEASE_REPO$PROOFAGENT_ARC  || clean_up 1
tar -xzf $PROOFAGENT_ARC || clean_up 1

export PROOFAGENTSYS="$WD/pod-agent"
export PATH=$PROOFAGENTSYS:$PATH 
export LD_LIBRARY_PATH=$PROOFAGENTSYS:$LD_LIBRARY_PATH

# Transmitting an executable through the InputSandbox does not preserve execute permissions
if [ ! -x $PROOFAGENTSYS/pod-agent ]; then 
    chmod +x $PROOFAGENTSYS/pod-agent
fi

# creating an empty proof.conf, so that xproof will be happy
touch $POD_PROOFCFG_FILE

# user defaults for ports
XRD_PORTS_RANGE_MIN=$($user_defaults -c $POD_CFG --section worker --key xrd_ports_range_min)
XRD_PORTS_RANGE_MAX=$($user_defaults -c $POD_CFG --section worker --key xrd_ports_range_max)
XPROOF_PORTS_RANGE_MIN=$($user_defaults -c $POD_CFG --section worker --key xproof_ports_range_min)
XPROOF_PORTS_RANGE_MAX=$($user_defaults -c $POD_CFG --section worker --key xproof_ports_range_max)

# we try for 5 times to detect/start xrd
# it is needed in case when several PoD workers are started in the same time on one machine
COUNT=0
MAX_COUNT=5
while [ "$COUNT" -lt "$MAX_COUNT" ]
  do
  # detecting whether xrd is running and on which ports xrd and xproof are listening
  xrd_detect
  return_val=$?
  if [ "X$return_val" != "X0" ]; then
      logMsg "problem to detect XRD/XPD ports. Exiting..."
      clean_up 1
  fi
  
  if [ -n "$XRD_PID" ]; then
      # use existing ports for xrd and xproof
      logMsg "found a running XRD instance with pid: "$XRD_PID
      POD_XRD_PORT_TOSET=$XRD_PORT
      POD_XPROOF_PORT_TOSET=$XPROOF_PORT
  else
      # if xrootd is not yet running on this machine for this user, try to start it
      logMsg "xrootd is not running yet on this machine for this user."
      POD_XRD_PORT_TOSET=`get_freeport $XRD_PORTS_RANGE_MIN $XRD_PORTS_RANGE_MAX`
      POD_XPROOF_PORT_TOSET=`get_freeport $XPROOF_PORTS_RANGE_MIN $XPROOF_PORTS_RANGE_MAX`
  fi
  logMsg "using XRD port: "$POD_XRD_PORT_TOSET
  logMsg "using XPROOF port: "$POD_XPROOF_PORT_TOSET
  
  # updating XRD configuration file. Needed even if another scrip has already started an xrootd process,
  # since we might want to use port's info somewhere else.
  regexp_xrd_port="s/\(xrd.port[[:space:]]*\)[0-9]*/\1$POD_XRD_PORT_TOSET/g"
  regexp_xproof_port="s/\(xrd.protocol[[:space:]]xproofd:\)[0-9]*/\1$POD_XPROOF_PORT_TOSET/g"
  sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" $WD/xpd.cf > $WD/xpd.cf.temp
  mv $WD/xpd.cf.temp $WD/xpd.cf

  # break the loop if xrootd is running already
  if [ -n "$XRD_PID" ]; then
      break
  fi

  logMsg "starting xrootd..."
  $(xrootd -c $WD/xpd.cf -b -l $WD/xpd.log)   
  #give xrootd some time to start
  sleep 3

  # loop counter
  COUNT=$(expr $COUNT + 1)
done

# detect that xrootd failed to start
XRD=$(pgrep -U $UID xrootd)
XRD_RET_VAL=$?
if [ "X$XRD_RET_VAL" = "X0" ]; then
    logMsg "checking XROOTD process: running..."
else
    logMsg "checking XROOTD process: is NOT running"
    clean_up 1
fi

logMsg "starting pod-agent..."
# start pod-agent
$PROOFAGENTSYS/pod-agent -c $POD_CFG -m worker --serverinfo $WD/server_info.cfg --proofport $POD_XPROOF_PORT_TOSET &

# wait for pod-agent's process
wait $!

logMsg "--- DONE ---"

# Exit
clean_up 0

