#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-01-19
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( PoD )

set(VERSION "2.1.4a")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "4a")


#
# Check if cmake has the required version
#
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )

# Install directory
if ( NOT $ENV{POD_LOCATION} STREQUAL "" )
  set (CMAKE_INSTALL_PREFIX "$ENV{POD_LOCATION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
else(NOT $ENV{POD_LOCATION} STREQUAL "" )
   set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/PoD/${VERSION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif ( NOT $ENV{POD_LOCATION} STREQUAL "" )


#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

#
# "additional" files
#
SET(POD_ADD_FILES
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/ReleaseNotes
    )

#
# "bin" files
#
SET(POD_BIN_FILES 
    ${CMAKE_SOURCE_DIR}/bin/pod-server
    ${CMAKE_SOURCE_DIR}/bin/pod-user-defaults-lite
    )

#
# "etc" files
#
SET(POD_ETC_FILES
     ${CMAKE_SOURCE_DIR}/etc/PoD.cfg
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF.jdl
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF_FZK.jdl
    )


include_directories( app/MiscCommon )

add_subdirectory ( app/pod-agent )
add_subdirectory ( app/pod-console )
add_subdirectory ( app/pod-user-defaults ) 




#
# configure files
# 
execute_process(COMMAND mktemp -d /tmp/PoD_XXXXXXXXXX
                OUTPUT_VARIABLE CMAKE_POD_TMP_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file( ${CMAKE_SOURCE_DIR}/etc/xpd.cf.in ${CMAKE_BINARY_DIR}/etc/xpd.cf @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/etc/PoD_env.sh.in ${CMAKE_BINARY_DIR}/PoD_env.sh @ONLY )

#
# Install
#
install(FILES ${POD_ADD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/LICENSE)
install(FILES ${CMAKE_BINARY_DIR}/etc/xpd.cf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/PoD_env.sh DESTINATION ${CMAKE_INSTALL_PREFIX})

install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.lsf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/PoDWorker.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)

install(PROGRAMS ${POD_BIN_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES ${POD_ETC_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)


 
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
