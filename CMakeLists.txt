#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2008-10-07
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2008-2009 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-agent )

set(VERSION "2.1.2b")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "2b")

# Check if cmake has the required version
# because of many bugs in FindBOOST of older versions of cmake we require cmake 2.6.4
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_SOURCE_DIR}/Doxyfile @ONLY )

#
# Options
#
#option(BUILD_DOCUMENTATION "Build source code documentation" OFF)


set( SOURCE_FILES
    ./src/
    ./src/Main.cpp
    ./src/AgentBase.cpp
    ./src/AgentServer.cpp
    ./src/AgentClient.cpp
    ./src/ThreadPool.cpp
    ./src/PROOFAgent.cpp
    ./src/Node.cpp
)


#
# search for Boost
#

find_package( Boost REQUIRED COMPONENTS thread program_options)

message(STATUS "  inc. dir.: ${Boost_INCLUDE_DIRS}")
message(STATUS "  lib. dir.: ${Boost_LIBRARY_DIRS}")
message(STATUS "  libs: ${Boost_LIBRARIES}")


#
# Documentation
#
#if(BUILD_DOCUMENTATION)
#    add_subdirectory(documentation)
#endif(BUILD_DOCUMENTATION)
									
#
## PROOFAgent executable
#
include_directories( ${CMAKE_BINARY_DIR} src Include ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(pod-agent ${SOURCE_FILES})

target_link_libraries (
    pod-agent
    ${Boost_LIBRARIES}
)

install(TARGETS pod-agent DESTINATION bin)


#
# Package
#

# excluding build directory
# excluding svn files and Eclipse project files
SET(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES
"/\\\\.;~$;/_;^${PROJECT_SOURCE_DIR}/build;CMakeFiles/;CMakeCache;gz$;Makefile\\\\.;.project;.cproject;.settings;cmake_install;CPack;;\\\\.svn/;Makefile$")
include(CPack)
