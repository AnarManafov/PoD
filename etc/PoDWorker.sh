#! /bin/bash

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
#        Copyright (c) 2007-2009 GSI GridTeam. All rights reserved.
#*************************************************************************/
#
# Arguments for the script:
#
# -r <ROOTSYS> : PoDWorker will not use its own version of ROOT on workers, instead it will use provided ROOTSYS
#
#

# ************************************************************************
# F U N C T I O N S
# ************************************************************************
# ***** Perform program exit housekeeping *****
clean_up()
{
    pkill -9 -U $UID xrootd
    pkill -9 -U $UID proofserv
    
# Archive and removing local proof directory
    _WD=`pwd`
    proof_dir="$_WD/proof"
    
    if [ -e "$proof_dir" ]; then
	# making an archive of proof logs
	# it will be transfered to a user
	tar -czvf proof_log.tgz $proof_dir
	echo "$proof_dir exists and will be deleted..."
	rm -rf $proof_dir
    fi
    
    exit $1
}
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
    XRD_PORTS=(`lsof -P -w -a -c xrootd -u $UID -i -n |  grep LISTEN  | sed -n -e 's/.*:\([0-9]*\).(LISTEN)/\1/p' | sort -b -n -u`)
    
    echo "PoD detected XRD port:"${XRD_PORTS[0]}
    echo "PoD detected XPROOF port:"${XRD_PORTS[1]}
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


# ************************************************************************
# ***** Arguments *****
WN_INSTALL_ROOT=True
WN_ROOTSYS=
while getopts "r:" Option
do
  case $Option in
    r) 
	  WN_INSTALL_ROOT=False 
	  WN_ROOTSYS=$OPTARG ;;
  esac
done



# ************************************************************************
# M A I N
# ************************************************************************

# handle signals
trap clean_up SIGHUP SIGINT SIGTERM 

# current working dir
WD=`pwd`
echo "Current working directory: $WD"
y=`eval ls -l`
echo "$y"

# Using eval to force variable substitution
# changing _G_WRK_DIR to a working directory in the following files:
eval sed -i 's%_G_WRK_DIR%$WD%g' ./xpd.cf
eval sed -i 's%_G_WRK_DIR%$WD%g' ./proofagent.client.cfg
# populating the tmp dir.
_TMP_DIR=`mktemp -d /tmp/PoDWorker_XXXXXXXXXX`
eval sed -i 's%_G_WORKER_TMP_DIR%$_TMP_DIR%g' ./xpd.cf

# host's CPU/instruction set
host_arch=`( uname -p ) 2>&1`
case "$host_arch" in
    i386|sparc|ppc|alpha|arm|mips)
	;;
    powerpc) # Darwin returns 'powerpc'
	host_arch=ppc
	;;
    *) # uname -p on Linux returns 'unknown' for the processor type,
      # OpenBSD returns 'Intel Pentium/MMX ("Genuine Intel" 586-class)'
	
      # Maybe uname -m (machine hardware name) returns something we
      # recognize.
	
      # x86/x86pc is used by QNX
	case "`( uname -m ) 2>&1`" in
	    i[3-9]86*|x86|x86pc|k5|k6|k6_2|k6_3|k6-2|k6-3|pentium*|athlon*|i586_i686|i586-i686) host_arch=x86 ;;
	    ia64) host_arch=ia64 ;;
	    x86_64) host_arch=x86_64 ;;
	    ppc) host_arch=ppc ;;
	    alpha) host_arch=alpha ;;
	    sparc*) host_arch=sparc ;;
	    9000*) host_arch=hppa ;;
	    arm*) host_arch=arm ;;
	    s390) host_arch=s390 ;;
	    s390x) host_arch=s390x ;;
	    mips) host_arch=mips ;;
	    *) host_arch=UNKNOWN ;;
	esac
	;;
esac

echo "*** host's CPU/instruction set: " $host_arch

case "$host_arch" in
    x86)
	PROOFAGENT_ARC="proofagent-2_0_0-x86-linux-gcc_4_1.tar.gz"
	ROOT_ARC="root_v5.18.00.Linux.slc4.gcc3.4.tar.gz" ;;
    x86_64)
        PROOFAGENT_ARC="proofagent-2_0_0-x86_64-linux-gcc_4_1.tar.gz"
        ROOT_ARC="root_v5.18.00.Linux.slc4_amd64.gcc3.4.tar.gz" ;;
esac

# ****************
# ***** ROOT *****
if [ "$WN_INSTALL_ROOT" = "True" ]; then
    wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/Binaries/$ROOT_ARC || clean_up 1
    tar -xzvf $ROOT_ARC || clean_up 1

    export ROOTSYS="/$WD/root"
    export PATH=$ROOTSYS/bin:$PATH
    export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
else
    export ROOTSYS=$WN_ROOTSYS
    export PATH=$ROOTSYS/bin:$PATH
    export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH 
fi



# ************************************************************************
# H E R E    U S E R S   C A N   D E C L A  R E   A   C U S T O M   E N V I R O N M E N T
# ************************************************************************


# ************************************************************************




# **********************
# ***** getting PROOFAgent from the repository site *****
wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/Binaries/$PROOFAGENT_ARC  || clean_up 1
tar -xzf $PROOFAGENT_ARC || clean_up 1

export PROOFAGENTSYS="/$WD/proofagent"
export PATH=$PROOFAGENTSYS/:$PATH 
export LD_LIBRARY_PATH=$PROOFAGENTSYS/:$LD_LIBRARY_PATH

# Transmitting an executable through the InputSandbox does not preserve execute permissions
if [ ! -x $PROOFAGENTSYS/proofagent ]; then 
    chmod +x $PROOFAGENTSYS/proofagent
fi

# creating an empty proof.conf, so that xproof will be happy
touch $WD/proof.conf

# detecting whether xrd is running and on whihc ports xrd and xproof are listning
xrd_detect
if [ -n "$XRD_PID" ]
then
    # use existing ports for xrd and xproof
    POD_XRD_PORT_TOSET=${XRD_PORTS[0]}
    POD_XPROOF_PORT_TOSET=${XRD_PORTS[1]}
else
    # TODO: get new free ports here and write to xrd config file
    XRD_PORTS_RANGE_MIN=20000
    XRD_PORTS_RANGE_MAX=21000
    XPROOF_PORTS_RANGE_MIN=21001
    XPROOF_PORTS_RANGE_MAX=22000
    POD_XRD_PORT_TOSET=`get_freeport $XRD_PORTS_RANGE_MIN $XRD_PORTS_RANGE_MAX`
    POD_XPROOF_PORT_TOSET=`get_freeport $XPROOF_PORTS_RANGE_MIN $XPROOF_PORTS_RANGE_MAX`
    echo "using XRD port:"$POD_XRD_PORT_TOSET
    echo "using XPROOF port:"$POD_XPROOF_PORT_TOSET
fi

# updating XRD configuration file
regexp_xrd_port="s/\(xrd.port[[:space:]]*\)[0-9]*/\1$POD_XRD_PORT_TOSET/g"
regexp_xproof_port="s/\(xrd.protocol[[:space:]]xproofd:\)[0-9]*/\1$POD_XPROOF_PORT_TOSET/g"
sed -e "$regexp_xrd_port" -e "$regexp_xproof_port" $WD/xpd.cf > $WD/xpd.cf.temp
mv $WD/xpd.cf.temp $WD/xpd.cf

#updating PROOFAgent configuration file
regexp="s/\(local_proofd_port=\)[0-9]*/\1$POD_XPROOF_PORT_TOSET/g"
sed -e "$regexp" $WD/proofagent.client.cfg > $WD/proofagent.client.cfg.temp
mv $WD/proofagent.client.cfg.temp $WD/proofagent.client.cfg

# starting xrootd
if [ -n "$XRD_PID" ]; then
    echo "using existing XRD instance..."
else
    echo "Starting xrootd..."
    xrootd -c $WD/xpd.cf -b -l $WD/xpd.log
# detect that xrootd failed to start
    sleep 10
    XRD=`pgrep -U $UID xrootd`
    XRD_RET_VAL=$?
    if [ "X$XRD_RET_VAL" = "X0" ]; then
	echo "XROOTD successful."
    else
	echo "problem to start xrootd. Exit code: $XRD_RET_VAL"
	clean_up 1
    fi
fi

# start proofagent
proofagent -c $WD/proofagent.client.cfg
RET_VAL=$?
if [ "X$RET_VAL" = "X0" ]; then
    echo "proofagent successful. Exit code: $RET_VAL"
else
    echo "cant start proofagent. Exit code: $RET_VAL"
    clean_up 1
fi

# Exit
clean_up 0
