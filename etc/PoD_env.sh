#!/usr/bin/env bash
##/************************************************************************/
##/*! \file PoD_env.sh.in
##  *//*
##
##         version number:     $LastChangedRevision$
##         created by:         Anar Manafov
##                             2008-02-06
##         last changed by:    $LastChangedBy$ $LastChangedDate$
##
##         Copyright (c) 2008-2010 GSI, Scientific Computing division. All rights reserved.
##*************************************************************************/
#=============================================================================
# ***** create_dir  *****
#=============================================================================
create_dir()
{
   if [ ! -d "$1" ]; then
      mkdir -p "$1"
   fi
}
#=============================================================================
# ***** MAIN  *****
#=============================================================================
# PoD Variables
if [ "x${BASH_ARGV[0]}" = "x" ]; then
    if [ ! -f PoD_env.sh ]; then
        echo ERROR: must "cd where/PoD/is" before calling ". PoD_env.sh" for this version of bash!
        POD_LOCATION=; export POD_LOCATION
        return 1
    fi
    POD_LOCATION="$PWD"; export POD_LOCATION
else
    # get param to "."
    THIS=$(dirname ${BASH_ARGV[0]})
    POD_LOCATION=$(cd ${THIS};pwd); export POD_LOCATION
fi

export PATH=$POD_LOCATION/bin:$PATH

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$POD_LOCATION/lib; export LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH=$POD_LOCATION/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

# Mac OS X
if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=$POD_LOCATION/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=$POD_LOCATION/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi


# local folder
eval LOCAL_POD="$HOME/.PoD"

# create local PoD directories
create_dir "$LOCAL_POD"
create_dir "$LOCAL_POD/etc"

# create a default configuration file if needed
POD_CFG=$(pod-user-defaults -p)
if [ -z "$POD_CFG" ]; then
   pod-user-defaults -d -c "$LOCAL_POD/PoD.cfg"
fi

# create working dir for custom locations
eval WORK_DIR=$(pod-user-defaults -V --key server.work_dir)
create_dir "$WORK_DIR"

