#! /bin/bash

#************************************************************************
#
# build.sh
# This script helps to build PAConsole
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2007-06-27
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2007 GSI GridTeam. All rights reserved.
#*************************************************************************
#
# REM: environment variables $QTDIR  $QTINC  $QTLIB must be predefined
# 

cd ..
$QTDIR/qmake PAConsole.pro
