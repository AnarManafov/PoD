#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2008-12-09
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2008 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console-lsf )


#set(CPACK_PACKAGE_VERSION_MAJOR "0")
#set(CPACK_PACKAGE_VERSION_MINOR "0")
#set(CPACK_PACKAGE_VERSION_PATCH "1")

# Check if cmake has the required version
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )


set( MOC_HDRS_lsf
    ./src/LSFDlg.h
    ./src/LSFJobSubmitter.h
    ./src/JobInfoItemModel.h
    ./src/JobsContainer.h
)

set( SOURCE_FILES_lsf
    ./src/LSFDlg.cpp
    ./src/LsfMng.cpp
    ./src/JobInfo.cpp
    ./src/JobInfoItemModel.cpp
    ./src/JobsContainer.cpp
)

set( UI_FILES_lsf
    ./res/wgLSF.ui
)

set( UI_SRSC_lsf 
    ./res/paconsole_lsf.qrc
)

QT4_WRAP_CPP( MOC_SRCS_lsf ${MOC_HDRS_lsf} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS_lsf ${UI_FILES_lsf} )
QT4_ADD_RESOURCES( RCS_SRCS_lsf  ${UI_SRSC_lsf} )


add_library(pod-console-lsf SHARED ${SOURCE_FILES_lsf} ${MOC_SRCS_lsf} ${RCS_SRCS_lsf} ${UI_HDRS_lsf})

set(CMAKE_CXX_FLAGS " -Wall")
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${LSF_INCLUDE_DIR} src )


target_link_libraries (
    pod-console-lsf
    ${LSF_LIBRARIES}
)

install(TARGETS pod-console-lsf DESTINATION plugins)

