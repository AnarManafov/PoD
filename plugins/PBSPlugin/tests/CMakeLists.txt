#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-03-22
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console-pbs-tests )


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

find_package( Boost 1.33.1 REQUIRED unit_test_framework)

#=============================================================================
add_executable(pod-console_test_pbs_submit test_submit.cpp  ${pod-console-pbs_SOURCE_DIR}/src/PbsMng.cpp)

include_directories( ${pod-console-pbs_SOURCE_DIR}/src ${PBS_INCLUDE_DIR} )

target_link_libraries (
    pod-console_test_pbs_submit
    ${Boost_LIBRARIES}
    ${PBS_LIBRARIES}
)

install(TARGETS pod-console_test_pbs_submit DESTINATION tests)

