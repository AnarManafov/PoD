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
export QTDIR=/usr/lib/qt4
export QTINC=/usr/include
export QTLIB=/usr/lib/qt4

# GAW
source /home/anar/GAW/bin/env.sh 

gmake distclean
$QTDIR/bin/qmake PAConsole.pro
gmake
