#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-10-13
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console-ge-tests )

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

find_package( Boost 1.33.1 REQUIRED unit_test_framework)

#=============================================================================
add_executable(pod-console_ge_test
               test_oge.cpp  
               ${pod-console-ge_SOURCE_DIR}/src/OgeMng.cpp )

include_directories( ${pod-console-ge_SOURCE_DIR}/src ${OGE_INCLUDE_DIR} )

target_link_libraries (
    pod-console_ge_test
    ${OGE_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install(TARGETS pod-console_ge_test DESTINATION tests)

