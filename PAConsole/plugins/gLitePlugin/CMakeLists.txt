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


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6.2 FATAL_ERROR )


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

QT4_WRAP_CPP( MOC_SRCS_gLite ${MOC_HDRS_gLite} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS_gLite ${UI_FILES_gLite} )
QT4_ADD_RESOURCES( RCS_SRCS_gLite  ${UI_SRSC_gLite} )


add_library(PAConsole_gLite SHARED ${SOURCE_FILES_gLite} ${MOC_SRCS_gLite} ${RCS_SRCS_gLite} ${UI_HDRS_gLite})

set(CMAKE_CXX_FLAGS "$ENV{GAW_CPPFLAGS} -Wall")										
include_directories( ${CMAKE_CURRENT_BINARY_DIR} src )

#set(CMAKE_CXX_FLAGS "-Wall")


target_link_libraries (
    PAConsole_gLite
  #  ${Boost_LIBRARIES}    
    glite-api-wrapper
)


#INSTALL(TARGETS PAConsole_gLite DESTINATION bin)

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
