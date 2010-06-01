#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2009-06-30
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-user-defaults )

set(VERSION "1.0.5a")
set(USER_DEFAULTS_CFG_VERSION "0.5")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "5a")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

if( NOT DEFINED IS_SET_POD_INSTALL_PREFIX )
  # Install directory
  if ( DEFINED ENV{POD_LOCATION} )
     set (CMAKE_INSTALL_PREFIX "$ENV{POD_LOCATION}" CACHE PATH
          "Install path prefix, prepended onto install directories." FORCE)
  endif ( DEFINED ENV{POD_LOCATION} )
endif( NOT DEFINED IS_SET_POD_INSTALL_PREFIX )

# Where to lookup modules
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# Options
#
#option(BUILD_DOCUMENTATION "Build source code documentation" OFF)

#
# configure files
# 
configure_file( ${pod-user-defaults_SOURCE_DIR}/src/version.h.in ${pod-user-defaults_BINARY_DIR}/version.h @ONLY )

set( SOURCE_FILES
    ./src/main.cpp
)

#
# search for Boost
#
message( STATUS "Looking for BOOST" )
find_package( Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
  message( STATUS "Looking for BOOST - YES" )
else(Boost_FOUND)
  message( FATAL_ERROR "Looking for BOOST - NO" )
endif(Boost_FOUND)
									
#
## pod-user-defaults executable
#
include_directories( ${pod-user-defaults_BINARY_DIR} src Include ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(pod-user-defaults ${SOURCE_FILES})

target_link_libraries (
    pod-user-defaults
    ${Boost_LIBRARIES}
)

install(TARGETS pod-user-defaults DESTINATION bin)

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

