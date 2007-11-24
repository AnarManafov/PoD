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

# ROOT
wget --tries=2 ftp://root.cern.ch/root/root_v5.17.04.Linux.slc4.gcc3.4.tar.gz || exit 1
tar -xzvf root_v5.17.04.Linux.slc4.gcc3.4.tar.gz || exit 1

export ROOTSYS="/$WD/root"
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

# BOOST
BOOST_VERSION="boost_1_34_1"
BOOST_INCLUDE="boost-1_34_1"
echo "Downloading BOOST src..."
`wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/ThirdParty/$BOOST_VERSION.tar.gz` || exit 1
echo "Unpacking BOOST src..."
`tar -xzf $BOOST_VERSION.tar.gz` || exit 1
pushd "$WD/$BOOST_VERSION"
echo "Configuring BOOST..."
gmake install || exit 1
popd

export LD_LIBRARY_PATH=$WD/BOOST/lib:$LD_LIBRARY_PATH

# Compiling 32bit version of PROOFAgent
export CXXFLAGS=-m32

# PROOFAgent
PA_VERSION="proofagent-1.0.3.1408"
echo "Downloading PROOAgent src..."
`wget --tries=2 http://www-linux.gsi.de/~manafov/D-Grid/Release/$PA_VERSION.tar.gz` || exit 1
echo "Unpacking PROOFAgent src..."
`tar -xzf $PA_VERSION.tar.gz` || exit 1
echo "Creating PROOFAgent dir..."
PA_DIR="$WD/PROOFAgent"
`mkdir $PA_DIR`
pushd "$WD/$PA_VERSION"
echo "Configuring PROOFAgent..."
./configure --prefix=$PA_DIR --with-boost-prefix=$WD/BOOST

RET_VAL=$?
if [ "X$RET_VAL" = "X0" ]; then
    echo "proofagent configuration successful. Exit code: $RET_VAL"
else
    cp "config.log" "$WD"
    exit 1
fi

gmake install
popd
`cp $PA_DIR/bin/proofagent $WD` || exit 1


# Transmitting an executable through the InputSandbox does not preserve execute permissions
if [ ! -x $WD/proofagent ]; then 
    chmod +x $WD/proofagent
fi

# creating an empty proof.conf, so that xproof will be happy
touch $WD/proof.conf

# start xrootd
echo "Starting xrootd..."
xrootd -c $WD/xpd.cf -b -l $WD/xpd.log

#
#
#RET_VAL=$?
#if [ "X$RET_VAL" = "X0" ]; then
#  echo "successful. Exit code: $RET_VAL"
#  
#else
#  echo "unsuccessful. Exit code: $RET_VAL"
#fi
#
#

# start proofagent
./proofagent -i client -c $WD/proofagent.cfg.xml
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
