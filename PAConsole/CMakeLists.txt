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
#        Copyright (c) 2008 GSI GridTeam. All rights reserved.
#*************************************************************************
project( PAConsole )


set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "6")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )


#
# Options
#
option(BUILD_DOCUMENTATION "Build source code documentation" OFF)
option(BUILD_GLITE_PLUGIN "Build gLite plug-in" OFF)

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

find_package( Qt4 REQUIRED )
set(QT_USE_QTXML 1)
#include_directories ( ${QT_INCLUDES} )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
include( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS ${UI_FILES} )
QT4_ADD_RESOURCES( RCS_SRCS  ${UI_RESOURCE} )

#
# search for Boost
#

# we set this manually if we want to use gLite plug-in,
# wich is linked with gLite version of BOOST :(
set(BOOST_ROOT "$ENV{BOOST_LOCATION}")

find_package( Boost 1.32 REQUIRED COMPONENTS serialization)

message(STATUS "  inc. dir.: ${Boost_INCLUDE_DIRS}")
message(STATUS "  lib. dir.: ${Boost_LIBRARY_DIRS}")
message(STATUS "  libs: ${Boost_LIBRARIES}")


#
# Documentation
#
if(BUILD_DOCUMENTATION)
    add_subdirectory(documentation)
endif(BUILD_DOCUMENTATION)
									
#
## PAConsole executable
#
include_directories( ${CMAKE_BINARY_DIR} src Include ${Boost_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable(PAConsole ${SOURCE_FILES} ${MOC_SRCS} ${RCS_SRCS} ${UI_HDRS})

target_link_libraries (
    PAConsole
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
# !* TODO *!

install(TARGETS PAConsole DESTINATION bin)


#
# Package
#

# excluding build directory
# excluding svn files and Eclipse project files
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}/*"
                              ".svn"
                              ".project"
                              ".cproject"
                              ".settings")
include(CPack)
