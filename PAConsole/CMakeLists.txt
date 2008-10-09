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


SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "6")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )


link_directories("$ENV{GAW_LOCATION}/lib")

#
# Options
#
OPTION(BUILD_DOCUMENTATION "Build source code documentation" OFF)

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
SET(QT_USE_QTXML 1)
#include_directories ( ${QT_INCLUDES} )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
include( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS ${UI_FILES} )
QT4_ADD_RESOURCES( RCS_SRCS  ${UI_RESOURCE} )

# search for Boost version 1.34
find_package( Boost 1.34 COMPONENTS serialization )
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

#
# Documentation
#
IF(BUILD_DOCUMENTATION)
    ADD_SUBDIRECTORY(documentation)
ENDIF(BUILD_DOCUMENTATION)

ADD_EXECUTABLE(PAConsole ${SOURCE_FILES} ${MOC_SRCS} ${RCS_SRCS} ${UI_HDRS})
											
include_directories( ${CMAKE_BINARY_DIR} src Include )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


target_link_libraries (
    PAConsole
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
    )

INSTALL(TARGETS PAConsole DESTINATION bin)

#
# Package
#

# excluding build directory
# excluding svn files and Eclipse project files
SET(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}/*"
                              ".svn"
                              ".project"
                              ".cproject"
                              ".settings")
message("CPACK_SOURCE_IGNORE_FILES = ${CPACK_SOURCE_IGNORE_FILES}")
INCLUDE(CPack)

#
# gLite plug-in
#
add_subdirectory ( plugins/gLitePlugin )

