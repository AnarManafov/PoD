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
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

#
# configure files
# 
execute_process(COMMAND mktemp -d /tmp/PoD_XXXXXXXXXX
                OUTPUT_VARIABLE CMAKE_POD_TMP_DIR)
configure_file( ${CMAKE_SOURCE_DIR}/etc/xpd.cf.in ${CMAKE_SOURCE_DIR}/etc/xpd.cf @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/etc/PoD_env.sh.in ${CMAKE_SOURCE_DIR}/etc/PoD_env.sh @ONLY )

#
# Install
#
install(FILES etc/xpd.cf DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
install(PROGRAMS etc/PoD_env.sh DESTINATION ${CMAKE_INSTALL_PREFIX})

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
