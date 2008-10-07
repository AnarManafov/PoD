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

#
# Options
#
OPTION(BUILD_DOCUMENTATION "Build source code documentation" OFF)

set( MOC_HDRS_gLite
    gliteplugin.h
)

set( SOURCE_FILES_gLite
        gliteplugin.cpp
)

set( UI_FILES_gLite
        gliteplugin.ui
)

#set( UI_RESOURCE_gLite 
#        ./res/paconsole.qrc
#)


find_package( Qt4 REQUIRED )
SET(QT_USE_QTXML 1)
#include_directories ( ${QT_INCLUDES} )
#The QT_USE_FILE macro is defined by FIND_PACKAGE that contains a path to the CMake script
include( ${QT_USE_FILE} )

QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS ${UI_FILES} )
#QT4_ADD_RESOURCES( RCS_SRCS  ${UI_RESOURCE} )

# search for Boost version 1.34
find_package( Boost 1.34 COMPONENTS thread )
#link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )


#add_library(PAConsole_gLite ${SOURCE_FILES} ${MOC_SRCS} ${RCS_SRCS} ${UI_HDRS})
add_library(PAConsole_gLite SHARED ${SOURCE_FILES_gLite} ${MOC_SRCS_gLite} ${UI_HDRS_gLite})
											
include_directories( ${CMAKE_BINARY_DIR} src Include )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{GAW_CPPFLAGS} -Wall")

target_link_libraries (
    PAConsole_gLite
    ${QT_LIBRARIES}
    glite-api-wrapper
#    ${Boost_LIBRARIES)
    )

#INSTALL(TARGETS PAConsole DESTINATION bin)

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
