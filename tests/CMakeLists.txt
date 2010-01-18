#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2009-12-25
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2009-2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-aget-tests )


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

find_package( Boost 1.33.1 REQUIRED unit_test_framework)

#=============================================================================
add_executable(pod-agent_test_ProtocolCommands Test_ProtocolCommands.cpp  ${CMAKE_SOURCE_DIR}/src/ProtocolCommands.cpp)

target_link_libraries (
    pod-agent_test_ProtocolCommands 
    ${Boost_LIBRARIES}
)

install(TARGETS pod-agent_test_ProtocolCommands DESTINATION tests)

#=============================================================================
set( SRC_PROTOCOL
    Test_Protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/Protocol.cpp
    ${CMAKE_SOURCE_DIR}/src/ProtocolCommands.cpp
)

add_executable(pod-agent_test_Protocol ${SRC_PROTOCOL})

target_link_libraries (
    pod-agent_test_Protocol 
    ${Boost_LIBRARIES}
)

install(TARGETS pod-agent_test_Protocol DESTINATION tests)

#=============================================================================
#
# configure files
# 
configure_file( ${CMAKE_SOURCE_DIR}/tests/xpd.cf.in ${CMAKE_SOURCE_DIR}/tests/xpd.cf @ONLY )

set( SRC_PROTOCOL
    Test_ProofStatusFile.cpp
    ${CMAKE_SOURCE_DIR}/src/ProofStatusFile.cpp
)

add_executable(pod-agent_test_ProofStatusFile ${SRC_PROTOCOL})

target_link_libraries (
    pod-agent_test_ProofStatusFile 
    ${Boost_LIBRARIES}
)

install(TARGETS pod-agent_test_Protocol DESTINATION tests)
install(FILES xpd.cf DESTINATION tests)

