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
#

# QT 4.2.3
export QTDIR=/usr/local/Trolltech/Qt-4.2.3
export QTINC=$QTDIR/include/Qt
export QTLIB=$QTDIR/lib

# GAW
source /home/anar/GAW/bin/env.sh 

rm -rf Makefile
$QTDIR/bin/qmake PAConsole.pro
gmake distclean
gmake
