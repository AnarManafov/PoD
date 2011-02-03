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
# Check if cmake has the required version
#
# we need cmake v. < v2.6.2 in order to use VERSION_EQUAL and other VERSION_*
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )
# Version
#
# take the version string from git and write it to a version file
# or take it from the version file if git is unavailable (a package distribution)
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND git describe --abbrev=4 HEAD
                  COMMAND sed -e "s/-/./g"
                  OUTPUT_VARIABLE POD_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  # remove leading "v"
  string(REGEX REPLACE "^v(.*)" "\\1" POD_VERSION ${POD_VERSION})
  execute_process( COMMAND bash -c "echo \"${POD_VERSION}\" > ${CMAKE_SOURCE_DIR}/etc/version" )

else( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND cat ${CMAKE_SOURCE_DIR}/etc/version
                  OUTPUT_VARIABLE POD_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

endif( EXISTS "${CMAKE_SOURCE_DIR}/.git" )
#
# Options
#
option(BUILD_DOCUMENTATION "Build documentation" OFF)
# tests
option(BUILD_TESTS "Build PoD unit tests" OFF)
# modules
option(BUILD_pod_agent "Build pod-agent" ON)
option(BUILD_pod_console "Build pod-console" ON)
option(BUILD_pod_ssh "Build pod-ssh" ON)
option(BUILD_pod_user_defaults "Build pod-user-defaults" ON)
# plug-ins
option(BUILD_GLITE_PLUGIN "Build gLite plug-in" ON)
option(BUILD_LSF_PLUGIN "Build LSF plug-in" ON)
option(BUILD_PBS_PLUGIN "Build PBS plug-in" ON)
option(BUILD_GE_PLUGIN "Build Grid Engine plug-in" ON)
#
# Documentation output directory
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
   set(DOC_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/documentation/api-docs)
   
endif(DOXYGEN_FOUND)
#
# Custom targets
#
if(DOXYGEN_FOUND)
   add_custom_target( api-docs COMMENT "Generating API docs" )
   add_dependencies( api-docs MiscCommon-api-docs pod-ssh-api-docs pod-agent-api-docs pod-console-api-docs pod-info-api-docs )

   add_custom_target( upload-api-docs 
       COMMAND rsync -avz -e ssh  ${DOC_OUTPUT_DIR} podwww@lxi001.gsi.de:~/web-docs/doc
       COMMENT "Upload API documentation to PoD's web server")
   add_dependencies( upload-api-docs api-docs )
endif(DOXYGEN_FOUND)
#
# Install directory
#
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "$ENV{HOME}/PoD/${POD_VERSION}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#
# This is needed to properly install PoD modules
#
set(IS_SET_POD_INSTALL_PREFIX 1 CACHE INTERNAL "")
#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}; ${CMAKE_SOURCE_DIR}/cmake")
#
# MiscCommon location
#
set(MiscCommon_LOCATION ${CMAKE_SOURCE_DIR}/app/MiscCommon)
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
    ${CMAKE_SOURCE_DIR}/bin/pod-check-update
    ${CMAKE_SOURCE_DIR}/bin/pod-prep-worker
    ${CMAKE_SOURCE_DIR}/bin/pod-submit
    )

SET(POD_BIN_PRIVATE_FILES
    ${CMAKE_SOURCE_DIR}/bin/private/pod-ssh-submit-worker
    ${CMAKE_SOURCE_DIR}/bin/private/pod-ssh-clean-worker
    ${CMAKE_SOURCE_DIR}/bin/private/pod-ssh-status-worker
    ${CMAKE_SOURCE_DIR}/bin/private/pod-remote-srv-info
    ${CMAKE_SOURCE_DIR}/bin/private/pod-ssh-tunnel
    )
#
# "plugins" files
#
SET(POD_CLI_PLUGINS_FILES 
    ${CMAKE_SOURCE_DIR}/plugins/cli/pod-lsf-submit
    ${CMAKE_SOURCE_DIR}/plugins/cli/pod-pbs-submit
    ${CMAKE_SOURCE_DIR}/plugins/cli/pod-ge-submit
    ${CMAKE_SOURCE_DIR}/plugins/cli/pod-condor-submit
    )
#
# "etc" files
#
SET(POD_ETC_FILES
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF.jdl
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF_FZK.jdl
     ${CMAKE_SOURCE_DIR}/etc/version
   )
#
# BUILD Sub Modules of PoD
#

#
# search for Boost
#
# boost needed by
# pod_agent: thread program_options filesystem (system)
# pod-console: threads serialization (+ some Header-Only libraries)
# pod-user-defaults: program_options
# unit tests: unit_test_framework
#
find_package(Boost 1.33.1 REQUIRED)
if(Boost_FOUND)
  set(local_boost_version "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

  set(Boost_Components thread program_options filesystem serialization)
  if(local_boost_version VERSION_GREATER "1.33.1")
      set(Boost_Components ${Boost_Components} system)
  endif(local_boost_version VERSION_GREATER "1.33.1")
  
  if( BUILD_TESTS)
    set(Boost_Components ${Boost_Components} unit_test_framework)
  endif( BUILD_TESTS)
  
  find_package( Boost 1.33.1 REQUIRED COMPONENTS  ${Boost_Components} )
else(Boost_FOUND)
    set(BUILD_pod_agent OFF)
    set(BUILD_pod_console OFF)
    set(BUILD_pod_user_defaults OFF)
    set(BUILD_pod_ssh OFF)
endif(Boost_FOUND)
#
# search for Qt
#
# Qt is needed by
# pod_console
#
find_package( Qt4 )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
include( ${QT_USE_FILE} )
if(QT4_FOUND)
  set(local_qt_version "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
  if(local_qt_version VERSION_LESS "4.4.2")
     message(WARNING " The version of Qt is too low. You will be not able to use PoD's GUI")
     set(BUILD_pod_console OFF)
  endif(local_qt_version VERSION_LESS "4.4.2")
else(QT4_FOUND)
  set(BUILD_pod_console OFF)
endif(QT4_FOUND)

  add_subdirectory( app/MiscCommon )

  if(BUILD_pod_agent)
    message(STATUS "Build pod-agent - YES")
    add_subdirectory ( app/pod-agent )
     set( PROTOCOL_FILES_LOCATION "${PROJECT_SOURCE_DIR}/app/pod-agent/src")
    message(STATUS "Build pod-info - YES")
    add_subdirectory( app/pod-info )
  else(BUILD_pod_agent)
    message(STATUS "Build pod-agent - NO")
  endif(BUILD_pod_agent)

  if(BUILD_pod_user_defaults)
    message(STATUS "Build pod-user-defaults - YES")
    add_subdirectory ( app/pod-user-defaults )
  else(BUILD_pod_user_defaults)
    message(STATUS "Build pod-user-defaults - NO")
  endif(BUILD_pod_user_defaults)

  if(BUILD_pod_ssh)
    message(STATUS "Build pod-ssh - YES")
    add_subdirectory ( app/pod-ssh )
  else(BUILD_pod_ssh)
    message(STATUS "Build pod-ssh - NO")
  endif(BUILD_pod_ssh)

  if(BUILD_pod_console)
    message(STATUS "Build pod-console - YES")
    add_subdirectory ( app/pod-console )
  else(BUILD_pod_console)
    message(STATUS "Build pod-console - NO")
  endif(BUILD_pod_console)

  if( NOT BUILD_pod_agent OR NOT BUILD_pod_user_defaults )
    message( FATAL_ERROR " Can't continue the build, since your environment doesn't meet the minimal requarements of PoD." )
  endif( NOT BUILD_pod_agent OR NOT BUILD_pod_user_defaults )
#
# configure files
# 
configure_file( ${CMAKE_SOURCE_DIR}/etc/PoD_env.sh.in ${CMAKE_BINARY_DIR}/etc/PoD_env.sh @ONLY )
#
# Install
#
install(FILES ${POD_ADD_FILES} DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/etc/xpd.cf.in DESTINATION etc)
install(PROGRAMS ${CMAKE_BINARY_DIR}/etc/PoD_env.sh DESTINATION .)
# special files, these needs to be executables
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.pbs DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.lsf DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.ge DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.condor DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/Job.condor.option DESTINATION etc)
install(PROGRAMS ${CMAKE_SOURCE_DIR}/etc/PoDWorker.sh DESTINATION etc)
# install bin files
install(PROGRAMS ${POD_BIN_FILES} DESTINATION bin)
install(PROGRAMS ${POD_BIN_PRIVATE_FILES} DESTINATION bin/private)
# install etc files
install(FILES ${POD_ETC_FILES} DESTINATION etc)
# install CLI plug-ins
install(PROGRAMS ${POD_CLI_PLUGINS_FILES} DESTINATION plugins/cli)
# install User's manual files
if( EXISTS  ${CMAKE_SOURCE_DIR}/documentation/html_help )
  install(FILES ${CMAKE_SOURCE_DIR}/documentation/index.html DESTINATION doc)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/documentation/html_help DESTINATION doc)
endif( EXISTS  ${CMAKE_SOURCE_DIR}/documentation/html_help )
# tests runner
if( BUILD_TESTS )
  install(PROGRAMS run_test.sh DESTINATION tests)
endif( BUILD_TESTS )

#
# Package
#
SET(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_SOURCE_IGNORE_FILES
    "bin/prep_gsi_bin.sh"
    "bin/prep_wrk_bins.sh"
    "prep_bins.sh"
    "inst_tmp"
    "PoD_env.sh$"
    "etc/xpd.cf$"
    "documentation/Diagrams"
    "app/pod-agent/doc"
    "app/pod-console/doc"
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

set( CPACK_SOURCE_PACKAGE_FILE_NAME "PoD-${POD_VERSION}-Source")

include(CPack)

