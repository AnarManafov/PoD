#! /bin/bash

#/************************************************************************/
#/**
# * @file gLitePROOF.sh
# * @brief gLitePROOF - a job script
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

# current working dir
WD=`pwd`
echo "Current working directory: $WD"
y=`eval ls -l`
echo "$y"

# Using eval to force variable substitution
# changing _G_WRK_DIR to a working directory in the following files:
eval sed -i 's%_G_WRK_DIR%$WD%g' ./xpd.cf
eval sed -i 's%_G_WRK_DIR%$WD%g' ./proofagent.cfg.xml

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
	PROOFAGENT_ARC="proofagent-1_0_4_1692-x86-linux-gcc_3_4.tar.gz"
	ROOT_ARC="root_v5.18.00.Linux.slc4.gcc3.4.tar.gz" ;;
x86_64)
        PROOFAGENT_ARC="proofagent-1_0_4_1692-x86_64-linux-gcc_3_4.tar.gz"
        ROOT_ARC="root_v5.18.00.Linux.slc4_amd64.gcc3.4.tar.gz" ;;
esac

# ROOT
wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/Binaries/$ROOT_ARC || exit 1
tar -xzvf $ROOT_ARC || exit 1

export ROOTSYS="/$WD/root"
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH


# PROOFAgent
wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/Binaries/$PROOFAGENT_ARC  || exit 1
tar -xzf $PROOFAGENT_ARC || exit 1

export PROOFAGENTSYS="/$WD/proofagent"
export PATH=$PROOFAGENTSYS/:$PATH 
export LD_LIBRARY_PATH=$PROOFAGENTSYS/:$LD_LIBRARY_PATH

# Transmitting an executable through the InputSandbox does not preserve execute permissions
if [ ! -x $PROOFAGENTSYS/proofagent ]; then 
    chmod +x $PROOFAGENTSYS/proofagent
fi

# creating an empty proof.conf, so that xproof will be happy
touch $WD/proof.conf

# start xrootd
echo "Starting xrootd..."
xrootd -c $WD/xpd.cf -b -l $WD/xpd.log

# detect that xrootd failed to start
sleep 10
XRD=`pgrep xrootd`
XRD_RET_VAL=$?
if [ "X$XRD_RET_VAL" = "X0" ]; then
    echo "XROOTD successful."
else
    echo "problem to start xrootd. Exit code: $XRD_RET_VAL"
    exit 1
fi

# start proofagent
proofagent -i client -c $WD/proofagent.cfg.xml
RET_VAL=$?
if [ "X$RET_VAL" = "X0" ]; then
    echo "proofagent successful. Exit code: $RET_VAL"    
else
    echo "cant start proofagent. Exit code: $RET_VAL"
fi

#
# daemon mode
#./proofagent -d -i client1 -p /tmp/ -c proofagent.cfg.xml --start
#

# sleep - in order to keep a job slot
#sleep 1200 # 1200 seconds

# Killing all xrootd in anyway (TODO: must be removed, needs an elegant solution)
pkill -9 xrootd
pkill -9 proofserv

# Removing local proof directory
# assuming that proof directory is located in the user's home
proof_dir="${HOME}/proof"
if [ -e "$proof_dir" ]; then
    echo "$proof_dir exists and will be deleted..."
    rm -rf $proof_dir
fi
