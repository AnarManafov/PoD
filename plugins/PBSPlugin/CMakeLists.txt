#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-03-22
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console-pbs )


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )


set( HDRS_pbs
     ./src/PbsMng.h
      ./src/JobInfo.h
      ./src/JobsContainer.h
    )

set( MOC_HDRS_pbs
    ./src/PbsDlg.h
    ./src/PbsJobSubmitter.h
    ./src/JobInfoItemModel.h
   )

set( SOURCE_FILES_pbs
    ./src/PbsDlg.cpp
    ./src/PbsMng.cpp
    ./src/set_attr.c
    ./src/JobInfo.cpp
    ./src/JobInfoItemModel.cpp
    ./src/JobsContainer.cpp
    ${pod-console_SOURCE_DIR}/src/ServerInfo.cpp 
)

set( UI_FILES_pbs
    ./res/wgPBS.ui
)

set( UI_SRSC_pbs 
    ./res/paconsole_pbs.qrc
)


QT4_WRAP_CPP( MOC_SRCS_pbs ${MOC_HDRS_pbs} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS_pbs ${UI_FILES_pbs} )
QT4_ADD_RESOURCES( RCS_SRCS_pbs  ${UI_SRSC_pbs} )

if (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-pbs SHARED
              ${SOURCE_FILES_pbs}
              ${HDRS_pbs}
              ${MOC_SRCS_pbs}
              ${MOC_HDRS_pbs}
              ${RCS_SRCS_pbs}
              ${UI_HDRS_pbs})
else (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-pbs SHARED
              ${SOURCE_FILES_pbs}
              ${MOC_SRCS_pbs}
              ${RCS_SRCS_pbs}
              ${UI_HDRS_pbs})
endif (CMAKE_GENERATOR STREQUAL "Xcode")


set(CMAKE_CXX_FLAGS " -Wall")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${PBS_INCLUDE_DIR} src )


target_link_libraries (
    pod-console-pbs
    ${PBS_LIBRARIES}
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
)

install(TARGETS pod-console-pbs DESTINATION plugins)

