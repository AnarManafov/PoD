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
#        Copyright (c) 2008 GSI GridTeam. All rights reserved.
#*************************************************************************
project( PROOFAgent )


set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

# Check if cmake has the required version
# because of many bugs in FindBOOST of older versions of cmake we require cmake 2.6.4
cmake_minimum_required( VERSION 2.6.4 FATAL_ERROR )

# Where to lookup modules
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

#
# Options
#
#option(BUILD_DOCUMENTATION "Build source code documentation" OFF)


set( SOURCE_FILES
    ./src/
    ./src/Main.cpp
    ./src/PacketForwarder.cpp
    ./src/PROOFAgent.cpp
    ./src/AgentImpl.cpp
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

add_executable(proofagent ${SOURCE_FILES})

target_link_libraries (
    proofagent
    ${Boost_LIBRARIES}
)

install(TARGETS proofagent DESTINATION bin)


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
