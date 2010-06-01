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

# This is needed if you want to use gLite plug-in and have several version of BOOST installed
#set( Boost_USE_MULTITHREADED OFF CACHE BOOL "BOOST" FORCE )
#set( Boost_USE_STATIC_LIBS ON CACHE BOOL "boost lit type" FORCE)
#set( Boost_COMPILER "-gcc" CACHE STRING "boost compiler prefix" FORCE)
#set( Boost_INCLUDE_DIR "/Users/anar/Documents/workspace/boost/include/boost-1_32" CACHE FILEPATH "boost inc dir." FORCE)
#set(BOOST_LIBRARYDIR "/Users/anar/Documents/workspace/boost/lib" CACHE PATH "boost libs" FORCE)

#
# Documentation
#
set( BUILD_DOCUMENTATION ON CACHE BOOL "Build source code documentation" FORCE )

