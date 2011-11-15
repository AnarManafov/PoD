#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2011-01-11
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2011 GSI, Scientific Computing division. All rights reserved.
#*************************************************************************
project( pod-info )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
#
# API Docs
#
if(DOXYGEN_FOUND)
   configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
   add_custom_target(pod-info-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
      COMMENT "Generating pod-info API docs")
endif(DOXYGEN_FOUND)

set( SOURCE_FILES
    ./src/main.cpp
    ./src/Server.cpp
    ./src/SrvInfo.cpp
)
set( SRC_HDRS
    ./src/Server.h
    ./src/SrvInfo.h
    ./src/Options.h
)

#
##  executable
#
include_directories( 
    ${PROJECT_BINARY_DIR}/src
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-info ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-info ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    pod-info
    pod_protocol
    proof_status_file
    SSHTunnel
    pod_sys_files
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install(TARGETS pod-info DESTINATION bin)

#
## Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build pod-info unit tests - YES")
#    add_subdirectory ( ${pod-info_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build pod-info unit tests - NO")
#endif (BUILD_TESTS)

