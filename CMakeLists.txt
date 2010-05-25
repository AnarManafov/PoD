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
#        Copyright (c) 2010 GSI, Scientific Computing group. All rights reserved.
#*************************************************************************
project( PoD )

#
# Version
#
set(VERSION "2.1.4c")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "4c")

#
# Check if cmake has the required version
#
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

#
# Options
#
option(GSI_BUILD "Build a GSI version" OFF)

#
# Install directory
#
if (DEFINED ENV{POD_LOCATION})
   set (CMAKE_INSTALL_PREFIX "$ENV{POD_LOCATION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
else (DEFINED ENV{POD_LOCATION})
   set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/PoD/${VERSION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif (DEFINED ENV{POD_LOCATION})

# This is needed to properly install PoD modules
set(IS_SET_POD_INSTALL_PREFIX 1 CACHE INTERNAL "")

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
    ${CMAKE_SOURCE_DIR}/bin/pod-prep-worker
    ${CMAKE_SOURCE_DIR}/bin/pod-user-defaults-lite
    ${CMAKE_SOURCE_DIR}/bin/pod-submit
    )

#
# "etc" files
#
SET(POD_ETC_FILES
     ${CMAKE_SOURCE_DIR}/etc/PoD.cfg
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF.jdl
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF_FZK.jdl
    )

if( NOT GSI_BUILD )
  message( STATUS " ***** Configuring pod-agent *****" )
  add_subdirectory ( app/pod-agent )
  message( STATUS " ***** Configuring pod-console *****" )
  add_subdirectory ( app/pod-console )
  message( STATUS " ***** Configuring pod-user-defaults *****" )
  add_subdirectory ( app/pod-user-defaults ) 
endif( NOT GSI_BUILD )

#
# GSI specific settings
#
if (GSI_BUILD)
    message (STATUS "Add GSI specific settings - YES")
    set(CMAKE_ADDITIONAL_LIB_PATH "export LD_LIBRARY_PATH=/LSF/lsf/lib:/misc/manafov/Qt/4.4.2_etch32/lib:$GLITE_PROOF_LOCATION/lib:$LD_LIBRARY_PATH" )
    
# TODO: automate the installation of gsi spec. libs und bins via git submodule
    set(GSI_ADD_BIN
        ${CMAKE_SOURCE_DIR}/bin/pod-agent
        ${CMAKE_SOURCE_DIR}/bin/pod-console
        ${CMAKE_SOURCE_DIR}/bin/pod-user-defaults 
    )
    set(GSI_ADD_LIB
        ${CMAKE_SOURCE_DIR}/lib/libglib-2.0.so.0
        ${CMAKE_SOURCE_DIR}/lib/libgthread-2.0.so.0
        ${CMAKE_SOURCE_DIR}/lib/libboost_filesystem-gcc-mt-1_33_1.so.1.33.1
        ${CMAKE_SOURCE_DIR}/lib/libboost_program_options-gcc-mt-1_33_1.so.1.33.1
        ${CMAKE_SOURCE_DIR}/lib/libboost_thread-gcc-mt-1_33_1.so.1.33.1 
    )
    install(PROGRAMS ${GSI_ADD_BIN} DESTINATION bin)
    install(FILES ${GSI_ADD_LIB} DESTINATION lib)
    install(FILES ${CMAKE_SOURCE_DIR}/plugins/libpod-console-lsf.so DESTINATION plugins )
endif (GSI_BUILD)

#
# configure files
# 
execute_process(COMMAND mktemp -d /tmp/PoD_XXXXXXXXXX
                OUTPUT_VARIABLE CMAKE_POD_TMP_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file( ${CMAKE_SOURCE_DIR}/etc/xpd.cf.in ${CMAKE_BINARY_DIR}/etc/xpd.cf @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/etc/PoD_env.sh.in ${CMAKE_BINARY_DIR}/PoD_env.sh @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/bin/pod-info.in ${CMAKE_BINARY_DIR}/bin/pod-info @ONLY )
#
# Install
#
install(FILES ${POD_ADD_FILES} DESTINATION .)
install(FILES ${CMAKE_BINARY_DIR}/etc/xpd.cf DESTINATION etc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/PoD_env.sh DESTINATION .)
install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/pod-info DESTINATION bin)
# special files, these needs to be executables
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.pbs DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.lsf DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/PoDWorker.sh DESTINATION etc)

install(PROGRAMS ${POD_BIN_FILES} DESTINATION bin)
install(FILES ${POD_ETC_FILES} DESTINATION etc)

#
# Package
#
# excluding build directory
# excluding svn files and Eclipse project files
SET(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_SOURCE_IGNORE_FILES
    "prep_gsi_bin.sh"
    "inst_tmp"
    "PoD_env.sh$"
    "bin/pod-info$"
    "etc/xpd.cf$"
    "documentation/Diagrams"
    "Logo"
    "/\\\\."
    "~$;/_"
    "^${PROJECT_SOURCE_DIR}/build"
    "CMakeFiles/"
    "CMakeCache"
    "gz$"
    "Makefile\\\\."
    ".project"
    ".cproject"
    ".settings"
    "cmake_install"
    "CPack"
    "\\\\.svn/"
    "Makefile$")

if (GSI_BUILD)
  set( CPACK_SOURCE_PACKAGE_FILE_NAME
      "PoD-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-GSI-etch32-bin")
endif(GSI_BUILD)

include(CPack)

if (GSI_BUILD)
  message("PoD is ready to be installed. Please execute \"make install\"")
endif(GSI_BUILD)
