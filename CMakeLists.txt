#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2008-10-07
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2008-2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console )

set(VERSION "1.2.2a")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "2a")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

# Where to lookup modules
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/version.h @ONLY )
configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )

#
# Options
#
option(BUILD_DOCUMENTATION "Build source code documentation" OFF)
option(BUILD_GLITE_PLUGIN "Build gLite plug-in" OFF)
option(BUILD_TESTS "Build pod-console unit tests" OFF)

set( MOC_HDRS
    ./src/PreferencesDlg.h
    ./src/MainDlg.h 
    ./src/ServerDlg.h 
    ./src/WorkersDlg.h
)

set( SOURCE_FILES
    ./src/PreferencesDlg.cpp  
    ./src/MainDlg.cpp
    ./src/ServerInfo.cpp 
    ./src/ServerDlg.cpp 
    ./src/WorkersDlg.cpp 
    ./src/main.cpp
)

set( UI_FILES
    ./res/wgPreferences.ui 
    ./res/maindlg.ui 
    ./res/wgServer.ui 
    ./res/wgWorkers.ui 
)

set( UI_RESOURCE 
        ./res/paconsole.qrc
)

set(QT_MIN_VERSION "4.4.2")
find_package( Qt4 REQUIRED )
#include_directories ( ${QT_INCLUDES} )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
include( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS ${UI_FILES} )
QT4_ADD_RESOURCES( RCS_SRCS  ${UI_RESOURCE} )

#
# search for Boost
#

find_package( Boost REQUIRED COMPONENTS serialization program_options)

message(STATUS "  inc. dir.: ${Boost_INCLUDE_DIRS}")
message(STATUS "  lib. dir.: ${Boost_LIBRARY_DIRS}")
message(STATUS "  libs: ${Boost_LIBRARIES}")


#
# Documentation
#
#if(BUILD_DOCUMENTATION)
#    add_subdirectory(documentation)
#endif(BUILD_DOCUMENTATION)
									
#
## pod-console executable
#
include_directories( ${PROJECT_BINARY_DIR} src Include ${Boost_INCLUDE_DIRS} )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-console ${SOURCE_FILES} ${MOC_SRCS} ${MOC_HDRS} ${RCS_SRCS} ${UI_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-console ${SOURCE_FILES} ${MOC_SRCS} ${RCS_SRCS})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    pod-console
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
)

#
# gLite plug-in
#
if (BUILD_GLITE_PLUGIN)
    message(STATUS "Looking for GAW")
    if ( "$ENV{GAW_LOCATION}" STREQUAL "" )
        message(STATUS "Looking for GAW - not found.")
        set(BUILD_GLITE_PLUGIN OFF)
    else( "$ENV{GAW_LOCATION}" STREQUAL "" )
        message(STATUS "Looking for GAW - found")
    endif( "$ENV{GAW_LOCATION}" STREQUAL "" )
endif (BUILD_GLITE_PLUGIN)

if (BUILD_GLITE_PLUGIN)
    # Build gLite plug-in
    add_subdirectory ( plugins/gLitePlugin )
    message (STATUS "Build gLite plug-in - YES")
else (BUILD_GLITE_PLUGIN)
    message (STATUS "Build gLite plug-in - NO")
endif (BUILD_GLITE_PLUGIN)

#
# LSF Plug-in
#
find_package( LSF REQUIRED )
if (LSF_FOUND)
    add_subdirectory ( plugins/LSFPlugin )
    message (STATUS "Build LSF plug-in - YES")
else (LSF_FOUND)
    message (STATUS "Build LSF plug-in - NO")    
endif (LSF_FOUND)   

#
# PBS Plug-in
#
find_package( PBS REQUIRED )
if (PBS_FOUND)
    add_subdirectory ( plugins/PBSPlugin )
    message (STATUS "Build PBS plug-in - YES")
    if (BUILD_TESTS)
       add_subdirectory ( plugins/PBSPlugin/tests )
       message (STATUS "Build tests of pbs plug-in")
    endif (BUILD_TESTS)
else (PBS_FOUND)
    message (STATUS "Build PBS plug-in - NO")    
endif (PBS_FOUND)

#
# Alien Plug-in
#
#find_package( Alien REQUIRED )
#if (ALIEN_FOUND)
#    add_subdirectory ( plugins/AlienPlugin )
#    message (STATUS "Build Alien plug-in - YES")
#else (ALIEN_FOUND)
#    message (STATUS "Build Alien plug-in - NO")
#endif (ALIEN_FOUND)


#
# Install Target
#
install(TARGETS pod-console DESTINATION bin)


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
