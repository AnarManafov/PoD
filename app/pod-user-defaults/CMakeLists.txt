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

set(USER_DEFAULTS_CFG_VERSION "0.8")

# Where to lookup modules
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${pod-user-defaults_SOURCE_DIR}/src/version.h.in ${pod-user-defaults_BINARY_DIR}/version.h @ONLY )

set( SOURCE_FILES
    ./src/main.cpp
)

set(Boost_filesystem_libs ${Boost_FILESYSTEM_LIBRARY})
if(local_boost_version VERSION_GREATER "1.33.1")
  set(Boost_filesystem_libs ${Boost_filesystem_libs} ${Boost_SYSTEM_LIBRARY})
endif(local_boost_version VERSION_GREATER "1.33.1")

set(Boost_libs_for_UD  ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_filesystem_libs})
								
#
## pod-user-defaults executable
#
include_directories( 
    ${pod-user-defaults_BINARY_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

add_executable(pod-user-defaults ${SOURCE_FILES})

target_link_libraries (
    pod-user-defaults
    ${Boost_libs_for_UD}
)

install(TARGETS pod-user-defaults DESTINATION bin)

