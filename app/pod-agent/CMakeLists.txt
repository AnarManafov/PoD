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
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#
# Custom targets
#
add_custom_target(pod-agent-api-docs doxygen ${PROJECT_BINARY_DIR}/Doxyfile
    COMMENT "Generating pod-agent API docs")

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

set(Boost_filesystem_libs ${Boost_FILESYSTEM_LIBRARY})
if(local_boost_version VERSION_GREATER "1.33.1")
  set(Boost_filesystem_libs ${Boost_filesystem_libs} ${Boost_SYSTEM_LIBRARY})
endif(local_boost_version VERSION_GREATER "1.33.1")

set(Boost_libs_for_agent ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_filesystem_libs})


#
## pod-agent executable
#
include_directories( 
    ${PROJECT_BINARY_DIR}/src 
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(pod-agent ${SOURCE_FILES})

target_link_libraries (
    pod-agent
    ${Boost_libs_for_agent}
)

install(TARGETS pod-agent DESTINATION bin)

#
## Unit tests
#
if (BUILD_TESTS)
    message (STATUS "Build pod-agent unit tests - YES")
    add_subdirectory ( ${pod-agent_SOURCE_DIR}/tests ) 
else (BUILD_TESTS)
    message (STATUS "Build pod-agent unit tests - NO")
endif (BUILD_TESTS)

