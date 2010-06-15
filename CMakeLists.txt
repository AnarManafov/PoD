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
# A GSI specific installation option. This type of installation
# will be removed as soon as GSI will upgrade to modern version of Linux ;) or will 
# get all required software installed GSI wide.
option(GSI_BUILD "Build a GSI version" OFF)
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

#
# Documentation output directory
#
set(DOC_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/documentation/api-docs)
#
# Custom targets
#
add_custom_target( api-docs COMMENT "Generating API docs" )
add_dependencies( api-docs MiscCommon-api-docs pod-ssh-api-docs pod-agent-api-docs pod-console-api-docs )

add_custom_target( upload-api-docs 
    COMMAND rsync -avz -e ssh  ${DOC_OUTPUT_DIR} podwww@lxi001.gsi.de:~/web-docs/doc
    COMMENT "Upload API documentation to PoD's web server")
add_dependencies( upload-api-docs api-docs )
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
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
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
    ${CMAKE_SOURCE_DIR}/bin/pod-prep-worker
    ${CMAKE_SOURCE_DIR}/bin/pod-user-defaults-lite
    ${CMAKE_SOURCE_DIR}/bin/pod-submit
    ${CMAKE_SOURCE_DIR}/bin/pod-ssh-submit-worker
    ${CMAKE_SOURCE_DIR}/bin/pod-ssh-clean-worker
    )
#
# "etc" files
#
SET(POD_ETC_FILES
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF.jdl
     ${CMAKE_SOURCE_DIR}/etc/gLitePROOF_FZK.jdl)
#
# BUILD Sub Modules of PoD
#
if( NOT GSI_BUILD )

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
     message(WARNING "The version of Qt is too low. You will be not able to use PoD's GUI")
     set(BUILD_pod_console OFF)
  endif(local_qt_version VERSION_LESS "4.4.2")
else(QT4_FOUND)
  set(BUILD_pod_console OFF)
endif(QT4_FOUND)

  add_subdirectory( app/MiscCommon )

  if(BUILD_pod_agent)
    message(STATUS "Build pod-agent - YES")
    add_subdirectory ( app/pod-agent )
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

  if( (NOT BUILD_pod_agent) OR (NOT BUILD_pod_user_defaults) )
    message( FATAL_ERROR "Can't continue the build, since your environment doesn't meet the minimal requarements of PoD." )
  endif( (NOT BUILD_pod_agent) OR (NOT BUILD_pod_user_defaults) )

endif( NOT GSI_BUILD )
#
# GSI specific settings
#
if (GSI_BUILD)
    message (STATUS "Add GSI specific settings - YES")
    set(CMAKE_ADDITIONAL_LIB_PATH "export LD_LIBRARY_PATH=/LSF/lsf/lib:/misc/manafov/Qt/4.4.2_etch32/lib:$GLITE_PROOF_LOCATION/lib:$LD_LIBRARY_PATH" )
    
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
# install bin files
install(PROGRAMS ${POD_BIN_FILES} DESTINATION bin)
# install etc files
install(FILES ${POD_ETC_FILES} DESTINATION etc)
# install User's manual files
if( EXISTS  ${CMAKE_SOURCE_DIR}/documentation/html_help )
  install(FILES ${CMAKE_SOURCE_DIR}/documentation/index.html DESTINATION doc)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/documentation/html_help DESTINATION doc)
endif( EXISTS  ${CMAKE_SOURCE_DIR}/documentation/html_help )
#
# Package
#
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

if(GSI_BUILD)
  set( CPACK_SOURCE_PACKAGE_FILE_NAME "PoD-${POD_VERSION}-GSI-etch32-bin")
else(GSI_BUILD)
  set( CPACK_SOURCE_PACKAGE_FILE_NAME "PoD-${POD_VERSION}-Source")
endif(GSI_BUILD)

include(CPack)

if (GSI_BUILD)
  message(" *********************************************************")
  message("This is a GSI specific PoD version and it is ready to be installed. Please execute \"make install\"")
  message(" *********************************************************")
endif(GSI_BUILD)
