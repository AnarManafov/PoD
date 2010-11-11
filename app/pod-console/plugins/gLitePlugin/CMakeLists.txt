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
project( pod-console-glite )


# build glite-api-wrapper first
message( STATUS " ***** Configuring glite-api-wrapper *****" )
add_subdirectory ( glite-api-wrapper )

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


add_library(pod-console-glite SHARED ${SOURCE_FILES_glite} ${MOC_SRCS_glite} ${RCS_SRCS_glite} ${UI_HDRS_glite})
									
include_directories( ${CMAKE_CURRENT_BINARY_DIR} src glite-api-wrapper/src ${GAW_REQUIRED_INCLUDES})
add_definitions(-DWANT_CLASSAD_NAMESPACE)

target_link_libraries (
    pod-console-glite 
    glite-api-wrapper
)

install(TARGETS pod-console-glite DESTINATION plugins/gui)

