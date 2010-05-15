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
#        Copyright (c) 2008-2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-agent )

#
# Version
#
set(VERSION "2.1.4a")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "4a")

#
# Check if cmake has the required version
#
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )

if( NOT DEFINED IS_SET_POD_INSTALL_PREFIX )
  # Install directory
  if ( DEFINED ENV{POD_LOCATION} )
     set (CMAKE_INSTALL_PREFIX "$ENV{POD_LOCATION}" CACHE PATH
          "Install path prefix, prepended onto install directories." FORCE)
  endif ( DEFINED ENV{POD_LOCATION} )
endif( NOT DEFINED IS_SET_POD_INSTALL_PREFIX )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )

#
# Options
#
option(BUILD_TESTS "Build pod-agent unit tests" OFF)


set( SOURCE_FILES
    ./src/Main.cpp
    ./src/AgentBase.cpp
    ./src/AgentServer.cpp
    ./src/AgentClient.cpp
    ./src/ThreadPool.cpp
    ./src/PROOFAgent.cpp
    ./src/Node.cpp
    ./src/Protocol.cpp
    ./src/ProtocolCommands.cpp
    ./src/ProofStatusFile.cpp
)

#
# search for Boost
#
find_package(Boost 1.33.1)
set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
if(local_boost_version VERSION_GREATER "1.33.1")
    FIND_PACKAGE(Boost 1.33.1 REQUIRED thread program_options filesystem system)
else(local_boost_version VERSION_GREATER "1.33.1")
    FIND_PACKAGE(Boost 1.33.1 REQUIRED thread program_options filesystem)
endif(local_boost_version VERSION_GREATER "1.33.1")

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
include_directories( ${PROJECT_BINARY_DIR}/src src Include ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(pod-agent ${SOURCE_FILES})

target_link_libraries (
    pod-agent
    ${Boost_LIBRARIES}
)

install(TARGETS pod-agent DESTINATION bin)
install(PROGRAMS run_test.sh DESTINATION bin)

#
## Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build pod-agent unit tests - YES")
    add_subdirectory ( tests ) 
else (BUILD_TESTS)
    message (STATUS "Build pod-agent unit tests - NO")
endif (BUILD_TESTS)


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
