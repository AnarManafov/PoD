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
project( PAConsole_glite )


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )


link_directories("$ENV{GAW_LOCATION}/lib")


set( MOC_HDRS_glite
    ./src/GridDlg.h
    ./src/JobSubmitter.h
    ./src/LogInfoDlg.h 
)

set( SOURCE_FILES_glite
    ./src/GridDlg.cpp
    ./src/LogInfoDlg.cpp
)

set( UI_FILES_glite
    ./res/wgGrid.ui
    ./res/wgLogInfo.ui
)

set( UI_SRSC_glite 
    ./res/paconsole_glite.qrc
)

QT4_WRAP_CPP( MOC_SRCS_glite ${MOC_HDRS_glite} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS_glite ${UI_FILES_glite} )
QT4_ADD_RESOURCES( RCS_SRCS_glite  ${UI_SRSC_glite} )


add_library(PAConsole_glite SHARED ${SOURCE_FILES_glite} ${MOC_SRCS_glite} ${RCS_SRCS_glite} ${UI_HDRS_glite})

set(CMAKE_CXX_FLAGS "$ENV{GAW_CPPFLAGS} -Wall")										
include_directories( ${CMAKE_CURRENT_BINARY_DIR} src )


target_link_libraries (
    PAConsole_glite 
    glite-api-wrapper
)

install(TARGETS PAConsole_glite DESTINATION plugins)

#
# Package
#

# excluding build directory
# excluding svn files and Eclipse project files
#set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}/*"
#                              ".svn"
#                              ".project"
#                              ".cproject"
#                              ".settings")
#message("CPACK_SOURCE_IGNORE_FILES = ${CPACK_SOURCE_IGNORE_FILES}")
#include(CPack)
