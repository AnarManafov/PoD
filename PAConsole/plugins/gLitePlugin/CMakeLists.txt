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
project( PAConsole_gLite )


SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )


link_directories("$ENV{GAW_LOCATION}/lib")


set( MOC_HDRS_gLite
    ./src/GridDlg.h
    ./src/JobSubmitter.h
    ./src/LogInfoDlg.h 
)

set( SOURCE_FILES_gLite
    ./src/GridDlg.cpp
    ./src/LogInfoDlg.cpp
)

set( UI_FILES_gLite
    ./res/wgGrid.ui
    ./res/wgLogInfo.ui
)

set( UI_SRSC_gLite 
        ./res/paconsole_glite.qrc
)


#find_package( Qt4 REQUIRED )
#SET(QT_USE_QTXML 1)
#include_directories ( ${QT_INCLUDES} )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
#include( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC_SRCS_gLite ${MOC_HDRS_gLite} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS_gLite ${UI_FILES_gLite} )
QT4_ADD_RESOURCES( RCS_SRCS_gLite  ${UI_SRSC_gLite} )

# search for Boost version 1.34
#find_package( Boost 1.34 COMPONENTS thread )
#link_directories ( ${Boost_LIBRARY_DIRS} )
#include_directories ( ${Boost_INCLUDE_DIRS} )

add_library(PAConsole_gLite SHARED ${SOURCE_FILES_gLite} ${MOC_SRCS_gLite} ${RCS_SRCS_gLite} ${UI_HDRS_gLite})
											
include_directories( ${CMAKE_CURRENT_BINARY_DIR} src  ${CMAKE_BINARY_DIR}/Include )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{GAW_CPPFLAGS} -Wall")

target_link_libraries (
    PAConsole_gLite
    glite-api-wrapper
    )

#INSTALL(TARGETS PAConsole DESTINATION bin)
