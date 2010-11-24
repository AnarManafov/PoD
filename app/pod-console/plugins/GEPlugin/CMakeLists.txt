#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-10-13
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI GridTeam. All rights reserved.
#*************************************************************************
project( pod-console-ge )

set( HDRS_oge
     ./src/OgeMng.h
      ./src/JobInfo.h
      ./src/JobsContainer.h
    )

set( MOC_HDRS_oge
    ./src/OgeDlg.h
    ./src/OgeJobSubmitter.h
    ./src/JobInfoItemModel.h
    ./src/JobsContainer.h
   )

set( SOURCE_FILES_oge
    ./src/OgeDlg.cpp
    ./src/OgeMng.cpp
    ./src/JobInfo.cpp
    ./src/JobsContainer.cpp
    ./src/JobInfoItemModel.cpp
    ${pod-console_SOURCE_DIR}/src/ServerInfo.cpp 
)

set( UI_FILES_oge
    ./res/wgOGE.ui
)

set( UI_SRSC_oge
    ./res/paconsole_oge.qrc
)


QT4_WRAP_CPP( GENERATED_MOC_SRCS_oge ${MOC_HDRS_oge} OPTIONS -f )
QT4_WRAP_UI( GENERATED_UI_HDRS_oge ${UI_FILES_oge} )
QT4_ADD_RESOURCES( GENERATED_RCS_SRCS_oge  ${UI_SRSC_oge} )

if (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-ge SHARED
              ${SOURCE_FILES_oge}
              ${HDRS_oge}
              ${MOC_HDRS_oge}
              ${UI_FILES_oge}
              ${GENERATED_MOC_SRCS_oge}
              ${GENERATED_RCS_SRCS_oge}
              ${GENERATED_UI_HDRS_oge})
else (CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(pod-console-ge SHARED
              ${SOURCE_FILES_oge}
              ${GENERATED_MOC_SRCS_oge}
              ${GENERATED_RCS_SRCS_oge}
              ${GENERATED_UI_HDRS_oge})
endif (CMAKE_GENERATOR STREQUAL "Xcode")


set(CMAKE_CXX_FLAGS " -Wall")

include_directories( ${pod-console_SOURCE_DIR}/src ${OGE_INCLUDE_DIR} ${PROJECT_BINARY_DIR} )


target_link_libraries (
    pod-console-ge
    ${OGE_LIBRARIES}
    ${QT_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
)

install(TARGETS pod-console-ge DESTINATION plugins/gui)

