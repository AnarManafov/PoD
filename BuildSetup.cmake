#
# to build PoD issue the following commands:
#  1) mkdir build
#  2) cd build
#  3) cmake -C ../BuildSetup.cmake ..
#  4) make install
#

#
# Install prefix
#
#SET (CMAKE_INSTALL_PREFIX "MY_PATH_HERE" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)

#
# BUILD TYPE
#
# set cmake build type, default value is: RelWithDebInfo
# possible options are: None Debug Release RelWithDebInfo MinSizeRel
set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )

#set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "This is useful for debugging only." FORCE)

# This is needed if you want to use gLite plug-in and have several version of BOOST installed
#set( Boost_USE_MULTITHREADED OFF CACHE BOOL "BOOST" FORCE )

#
# Documentation
#
# set( BUILD_DOCUMENTATION ON CACHE BOOL "Build source code documentation" FORCE )

#
# Tests
#
set( BUILD_TESTS ON CACHE BOOL "Build PoD tests" FORCE )

#
# gLite plug-in
#
#set( BUILD_GLITE_PLUGIN OFF CACHE BOOL "Build gLite plug-in" FORCE )

#
# LSF plug-in
#
#set( BUILD_LSF_PLUGIN OFF CACHE BOOL "Build LSF plug-in" FORCE )
#set( LSF_PREFIX "/home/anar/LSF" CACHE STRING "LSF prefix" FORCE)

#
# PBS plug-in
#
#set( BUILD_PBS_PLUGIN OFF CACHE BOOL "Build PBS plug-in" FORCE )

#
# Grid Engine (SGE/OGE) plug-in
#
#set( BUILD_GE_PLUGIN OFF CACHE BOOL "Build Grid Engine plug-in" FORCE )

