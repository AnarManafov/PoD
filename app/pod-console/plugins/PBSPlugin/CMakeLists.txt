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

set( HDRS_pbs
     ./src/PbsMng.h
      ./src/JobInfo.h
      ./src/JobsContainer.h
    )

set( MOC_HDRS_pbs
    ./src/PbsDlg.h
    ./src/PbsJobSubmitter.h
    ./src/JobInfoItemModel.h
    ./src/JobsContainer.h
   )

set( SOURCE_FILES_pbs
    ./src/PbsDlg.cpp
    ./src/PbsMng.cpp
    ./src/set_attr.c
    ./src/JobInfo.cpp
    ./src/JobsContainer.cpp
    ./src/JobInfoItemModel.cpp
    ${pod-console_SOURCE_DIR}/src/ServerInfo.cpp 
)

set( UI_FILES_pbs
    ./res/wgPBS.ui
)

set( UI_SRSC_pbs 
    ./res/paconsole_pbs.qrc
)


QT4_WRAP_CPP( GENERATED_MOC_SRCS_pbs ${MOC_HDRS_pbs} OPTIONS -f )
QT4_WRAP_UI( GENERATED_UI_HDRS_pbs ${UI_FILES_pbs} )
QT4_ADD_RESOURCES( GENERATED_RCS_SRCS_pbs  ${UI_SRSC_pbs} )

if (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-pbs SHARED
              ${SOURCE_FILES_pbs}
              ${HDRS_pbs}
              ${MOC_HDRS_pbs}
              ${UI_FILES_pbs}
              ${GENERATED_MOC_SRCS_pbs}
              ${GENERATED_RCS_SRCS_pbs}
              ${GENERATED_UI_HDRS_pbs})
else (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-pbs SHARED
              ${SOURCE_FILES_pbs}
              ${GENERATED_MOC_SRCS_pbs}
              ${GENERATED_RCS_SRCS_pbs}
              ${GENERATED_UI_HDRS_pbs})
endif (CMAKE_GENERATOR STREQUAL "Xcode")


set(CMAKE_CXX_FLAGS " -Wall")

include_directories( ${pod-console_SOURCE_DIR}/src ${PBS_INCLUDE_DIR} ${PROJECT_BINARY_DIR} )


target_link_libraries (
    pod-console-pbs
    ${PBS_LIBRARIES}
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
)

install(TARGETS pod-console-pbs DESTINATION plugins)

