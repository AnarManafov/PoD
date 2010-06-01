#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-05-17
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI, Scientific Computing group. All rights reserved.
#*************************************************************************
project( pod-ssh )

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
option(BUILD_TESTS "Build pod-ssh unit tests" OFF)


set( SOURCE_FILES
    ./src/main.cpp
 )

#
# search for Boost
#
find_package(Boost 1.33.1 REQUIRED thread program_options)
message(STATUS "  inc. dir.: ${Boost_INCLUDE_DIRS}")
message(STATUS "  lib. dir.: ${Boost_LIBRARY_DIRS}")
message(STATUS "  libs: ${Boost_LIBRARIES}")

include_directories( ${PROJECT_BINARY_DIR}/src src Include ${Boost_INCLUDE_DIRS} )

#
## pod-ssh executable
#
add_executable(pod-ssh ${SOURCE_FILES})

target_link_libraries (
    pod-ssh
    ${Boost_LIBRARIES}
)

install(TARGETS pod-ssh DESTINATION bin)
install(PROGRAMS run_test.sh DESTINATION bin)

#
## Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build pod-ssh unit tests - YES")
    add_subdirectory ( tests ) 
else (BUILD_TESTS)
    message (STATUS "Build pod-ssh unit tests - NO")
endif (BUILD_TESTS)

