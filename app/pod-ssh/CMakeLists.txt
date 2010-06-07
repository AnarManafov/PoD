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
# Source files
#
set( SOURCE_FILES
    ./src/main.cpp
    ./src/config.cpp
)

include_directories(
    ${PROJECT_BINARY_DIR}/src
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS} )

#
## pod-ssh executable
#
add_executable(pod-ssh ${SOURCE_FILES})

target_link_libraries (
    pod-ssh
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install(TARGETS pod-ssh DESTINATION bin)
install(PROGRAMS run_test.sh DESTINATION bin)

#
## Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build pod-ssh unit tests - YES")
    add_subdirectory ( ${pod-ssh_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build pod-ssh unit tests - NO")
endif (BUILD_TESTS)
