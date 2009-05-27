#
# to build vsual-netstat:
#  1) mkdir build
#  2) cd build
#  3) cmake -C ../BuildSetup.cmake ..
#  4) gmake install
#

#
# General Options
#

# Install directory
if ( NOT $ENV{GLITE_PROOF_LOCATION} STREQUAL "" )
set (CMAKE_INSTALL_PREFIX "$ENV{GLITE_PROOF_LOCATION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif ( NOT $ENV{GLITE_PROOF_LOCATION} STREQUAL "" )

# set cmake build type, default value is: RelWithDebInfo
# possible options are: None Debug Release RelWithDebInfo MinSizeRel
set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )
#set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "This is useful for debugging only." FORCE)

# This is needed if you want to use gLite plug-in and have several version of BOOST installed
#set( Boost_USE_MULTITHREADED OFF CACHE BOOL "BOOST" FORCE )

#
# Documentation
#
set( BUILD_DOCUMENTATION ON CACHE BOOL "Build source code documentation" FORCE )

