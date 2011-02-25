#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2011-02-25
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2011 GSI, Scientific Computing devision. All rights reserved.
#*************************************************************************
project( pod-remote )

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
   add_custom_target(pod-remote-api-docs ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
      COMMENT "Generating pod-remote API docs")
endif(DOXYGEN_FOUND)

set( SOURCE_FILES
    ./src/main.cpp
    ./src/Environment.cpp
    ./src/Server.cpp
    ./src/SrvInfo.cpp
    ./src/SSHTunnel.cpp
    ${PROTOCOL_FILES_LOCATION}/Protocol.cpp
    ${PROTOCOL_FILES_LOCATION}/ProtocolCommands.cpp
    ${PROTOCOL_FILES_LOCATION}/ProofStatusFile.cpp
)
set( SRC_HDRS
    ./src/Environment.h
    ./src/Server.h
    ./src/SrvInfo.h
    ./src/Options.h
    ./src/SSHTunnel.h
    ${PROTOCOL_FILES_LOCATION}/Protocol.h
    ${PROTOCOL_FILES_LOCATION}/ProtocolCommands.h
    ${PROTOCOL_FILES_LOCATION}/ProofStatusFile.h
)

set(Boost_filesystem_libs ${Boost_FILESYSTEM_LIBRARY})
if(local_boost_version VERSION_GREATER "1.33.1")
  set(Boost_filesystem_libs ${Boost_filesystem_libs} ${Boost_SYSTEM_LIBRARY})
endif(local_boost_version VERSION_GREATER "1.33.1")

set(Boost_libs_for_pod_remote ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_filesystem_libs})

#
##  executable
#
include_directories( 
    ${PROJECT_BINARY_DIR}/src
    ${MiscCommon_LOCATION}
    ${PROTOCOL_FILES_LOCATION}
    ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-remote ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-remote ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    pod-remote
    ${Boost_libs_for_pod_remote}
)

install(TARGETS pod-remote DESTINATION bin)

#
## Unit tests
#
#if (BUILD_TESTS)
#    message (STATUS "Build pod-remote unit tests - YES")
#    add_subdirectory ( ${pod-remote_SOURCE_DIR}/tests ) 
#else (BUILD_TESTS)
#    message (STATUS "Build pod-remote unit tests - NO")
#endif (BUILD_TESTS)

