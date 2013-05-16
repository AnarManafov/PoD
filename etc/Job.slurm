#!/usr/bin/env bash
#/************************************************************************/
##/**
## * @file Job.slurm
## * @brief a SLURM job script
## * @author Anar Manafov A.Manafov@gsi.de
## *//*
##
##        version number:    $LastChangedRevision$
##        created by:        Anar Manafov
##                           2013-04-15
##        last changed by:   $LastChangedBy$ $LastChangedDate$
##
##         Copyright (c) 2013 GSI, Scientific Computing division. All rights reserved.
##*************************************************************************/

######################################################
# The following variables are expected to set:
# $POD_WRK_SCRIPT
# $POD_UI_LOCATION
# $POD_UI_LOG_LOCATION
WRK_SCRIPT=$(basename "$POD_WRK_SCRIPT")
UI_LOG_DIR=$POD_UI_LOG_LOCATION/$PARENT_PBS_JOBID
###############################################################
#
#    Function: Transfer files from WN to UI.                  
#
###############################################################
stageout()
{
   eval upload_job_log=$($WRK_DIR/pod-user-defaults -c $WRK_DIR/PoD.cfg --key slurm_plugin.upload_job_log)

   if [ "$upload_job_log" = "1" ]; then
      echo "Staging the log files out..."
      echo "log dir: "$UI_LOG_DIR

      cp $WRK_DIR/proof_log.tgz $UI_LOG_DIR/proof_log.$SLURM_JOBID.tgz
      cp $WRK_DIR/xpd.log $UI_LOG_DIR/xpd.$SLURM_JOBID.log
      cp $WRK_DIR/pod-agent.client.log $UI_LOG_DIR/pod-agent.client.$SLURM_JOBID.log 
   fi
}

###############################################################
#
# create a working folder
#
###############################################################
WRK_DIR=$(mktemp -d /tmp/PoD_XXXXXXXXXX)
if [ -z $WRK_DIR ]; then
   echo "Error: Can't create a working directory."
   exit 1
fi
echo "working directory: " $WRK_DIR
cd $WRK_DIR || exit 1
# global variables
# we need to define lib path to ensure that we use proper libraries
export PATH=$WRK_DIR:$PATH 
export LD_LIBRARY_PATH=$WRK_DIR:$LD_LIBRARY_PATH
################################################################
#
# Execute Wrk Script
#
###############################################################
$POD_WRK_SCRIPT
###############################################################
#
# get all logs
#
###############################################################
stageout
###############################################################
#
# remove working folder
#
###############################################################
if [[ -d $WRK_DIR ]]; then
   rm -rf $WRK_DIR
fi

exit 0
