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
#        Copyright (c) 2010-2011 GSI, Scientific Computing devision. All rights reserved.
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
# Keep the version file in the source directory to satisfy the source package.
set( POD_VERSION_FILE ${CMAKE_SOURCE_DIR}/etc/version )
if( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND git describe --abbrev=4 HEAD
                  COMMAND sed -e "s/-/./g"
                  OUTPUT_VARIABLE POD_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  # remove leading "v"
  string(REGEX REPLACE "^v(.*)" "\\1" POD_VERSION ${POD_VERSION})
  execute_process( COMMAND bash -c "echo \"${POD_VERSION}\" > ${POD_VERSION_FILE}" )

else( EXISTS "${CMAKE_SOURCE_DIR}/.git" )

  execute_process(COMMAND cat ${POD_VERSION_FILE}
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
   add_dependencies( api-docs
                     MiscCommon-api-docs
                     pod-ssh-api-docs
                     pod-agent-api-docs
                     pod-console-api-docs
                     pod-info-api-docs
                     pod-remote-api-docs )

   add_custom_target( upload-api-docs 
       COMMAND rsync -avz -e ssh  ${DOC_OUTPUT_DIR} podwww@lxi001.gsi.de:~/web-docs/doc
       COMMENT "Upload API documentation to PoD's web server")
   add_dependencies( upload-api-docs api-docs )
endif(DOXYGEN_FOUND)
# a wn bin package custom target
# Because of $<TARGET_FILE:tgt> it works only in cmake 2.8.4
if( ${CMAKE_VERSION} VERSION_GREATER 2.8.3 )
   if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set(TRG_ARCH i386)
   else( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set(TRG_ARCH amd64)
   endif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
   
   set( WN_PKG_NAME "pod-wrk-bin-${POD_VERSION}-${CMAKE_SYSTEM_NAME}-${TRG_ARCH}" )
   set( WN_PKG_DIR "${CMAKE_BINARY_DIR}/pod-wrk-bin" )
   set( WN_PKG_NAME_ARCH "${WN_PKG_NAME}.tar.gz" )
   set( WN_PKG_REMOTE_DIR "/u/podwww/web-docs/releases/add/${POD_VERSION}" )

   if(APPLE)
     set( lib_cmd "bash" )
     set( lib_cmd_args "-c"; "otool -L ${WN_PKG_DIR}/pod-agent  | grep boost | awk '{ system( \"cp \" $1 \" ${WN_PKG_DIR}\") }'" )
   elseif(UNIX)
     set( lib_cmd "bash" )
     set( lib_cmd_args "-c"; "ldd ${WN_PKG_DIR}/pod-agent | grep boost | awk '{ system( \"cp \" $3 \" ${WN_PKG_DIR}\") }'" )
   endif(APPLE)
   
   add_custom_target( wn_bin
                      COMMAND -mkdir -p ${WN_PKG_DIR}
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pod-agent> "${WN_PKG_DIR}"
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pod-user-defaults> "${WN_PKG_DIR}"
		      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pod_protocol> "${WN_PKG_DIR}"
                      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:proof_status_file> "${WN_PKG_DIR}"
                      COMMAND ${lib_cmd} ${lib_cmd_args}
                      COMMAND ${CMAKE_COMMAND} -E tar czf ${WN_PKG_NAME_ARCH} "${WN_PKG_DIR}"
		      COMMAND ssh podwww@lxi001 "mkdir -p ${WN_PKG_REMOTE_DIR} && chmod go+xr ${WN_PKG_REMOTE_DIR}"                      
                      COMMAND scp -p ${WN_PKG_NAME_ARCH} podwww@lxi001:${WN_PKG_REMOTE_DIR} 
                      COMMENT "Generate a WN binary package" 
                      VERBATIM )
   add_dependencies( wn_bin
                     pod-agent
                     pod-user-defaults )
endif(${CMAKE_VERSION} VERSION_GREATER 2.8.3)
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
set(MiscCommon_LOCATION
    ${MiscCommon_LOCATION}
    ${MiscCommon_LOCATION}/pod_protocol
    ${MiscCommon_LOCATION}/pipe_log_engine
    ${MiscCommon_LOCATION}/proof_status_file
)
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
     ${POD_VERSION_FILE}
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
  # force to use the version 2 of the filesystem library, since
  # we must support boost 1.33.1.
  # TODO: implement support of the version 3
  add_definitions(-DBOOST_FILESYSTEM_VERSION=2)
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
    message(STATUS "Build pod-info - YES")
    add_subdirectory( app/pod-info )
    message(STATUS "Build pod-remote - YES")
    add_subdirectory( app/pod-remote )    
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
  install(FILES ${CMAKE_SOURCE_DIR}/documentation/html_help/PoD.pdf DESTINATION doc)
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
    "inst_tmp"
    "PoD_env.sh$"
    "etc/xpd.cf$"
    "documentation/Diagrams"
    "documentation/html_help/.*\\\\.html$"
    "documentation/html_help/.*\\\\.png$"
    "documentation/html_help/.*\\\\.css$"
    "documentation/html_help/.*\\\\.ps$"
    "documentation/api-docs"
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
    "Makefile$"
    "PoDFiles.txt$")

set( CPACK_SOURCE_PACKAGE_FILE_NAME "PoD-${POD_VERSION}-Source")

include(CPack)

