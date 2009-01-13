#! /bin/bash
##/************************************************************************/
##/*! \file PoD_env.sh.template
##  *//*
##
##         version number:     $LastChangedRevision$
##         created by:         Anar Manafov
##                             2008-02-06
##         last changed by:    $LastChangedBy$ $LastChangedDate$
##
##         Copyright (c) 2008 GSI GridTeam. All rights reserved.
##*************************************************************************/

export GLITE_PROOF_LOCATION=_G_WRK_DIR
export PATH=$GLITE_PROOF_LOCATION/bin:$PATH

# sourcing GAW environment, if installed
if [[ -f $GLITE_PROOF_LOCATION/bin/gaw_env.sh ]]; then
    source $GLITE_PROOF_LOCATION/bin/gaw_env.sh
fi
