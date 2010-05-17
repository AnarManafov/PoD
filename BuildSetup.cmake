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


# set cmake build type, default value is: RelWithDebInfo
# possible options are: None Debug Release RelWithDebInfo MinSizeRel
set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )
#set(CMAKE_VERBOSE_MAKEFILE TRUE CACHE BOOL "This is useful for debugging only." FORCE)

# build unit tests
set( BUILD_TESTS ON CACHE BOOL "Build pod-agent unit tests" FORCE )

#
# Documentation
#
set( BUILD_DOCUMENTATION ON CACHE BOOL "Build source code documentation" FORCE )

