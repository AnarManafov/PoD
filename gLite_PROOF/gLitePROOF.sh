#! /bin/bash

#/************************************************************************/
#/**
# * @file gLitePROOF.sh
# * @brief $$File comment$$
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
# Using eval to force variable substitution
# changing <WD> to a working directory in the following files:
eval sed -i 's%\<WD\>%$WD%g' xpd.cfg
eval sed -i 's%\<WD\>%$WD%g' proofagent.cfg.xml


# ROOT
export ROOTSYS=/usr/ROOT/5.14.00
export PATH=$ROOTSYS/bin:$PATH
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH


# start xrootd
echo "Starting xrootd..."
xrootd -c ~/xpd.cf -b -l ~/xpd.log

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
./proofagent -i client -c ~/proofagent.cfg.xml --start
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