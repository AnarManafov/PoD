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
project( pod-console )

# Where to lookup modules
set (CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/plugins/gLitePlugin/glite-api-wrapper/cmake")

#
# configure files
# 
configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
configure_file( ${PROJECT_SOURCE_DIR}/Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY )
#
# Custom targets
#
add_custom_target(pod-console-api-docs doxygen ${PROJECT_BINARY_DIR}/Doxyfile
    COMMENT "Generating pod-console API docs")

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


QT4_WRAP_CPP( MOC_SRCS ${MOC_HDRS} OPTIONS -f )
QT4_WRAP_UI( UI_HDRS ${UI_FILES} )
QT4_ADD_RESOURCES( RCS_SRCS  ${UI_RESOURCE} )

#
# Add macros
#
if( CMAKE_BUILD_TYPE STREQUAL "Release" )
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
  add_definitions(-DQT_NO_DEBUG)
  add_definitions(-DNDEBUG)
endif()

#
## pod-console executable
#
include_directories( 
    ${PROJECT_BINARY_DIR}/src 
    ${PROJECT_BINARY_DIR}
    ${MiscCommon_LOCATION}
    ${Boost_INCLUDE_DIRS}
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-console ${SOURCE_FILES} ${MOC_SRCS} ${MOC_HDRS} ${RCS_SRCS} ${UI_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-console ${SOURCE_FILES} ${MOC_SRCS} ${RCS_SRCS} ${UI_HDRS})
endif (CMAKE_GENERATOR STREQUAL "Xcode")

target_link_libraries (
    pod-console
    ${QT_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
)

#
# gLite plug-in
#
if( BUILD_GLITE_PLUGIN )
  message (STATUS "Looking for gLite API")
  find_package( gLite )
  IF (HAVE_gLite)
     message (STATUS "Looking for gLite API - YES")
     message (STATUS "Build gLite plug-in")
     add_subdirectory ( plugins/gLitePlugin )
     message (STATUS "Build gLite plug-in - YES")
  else (HAVE_gLite)
      message (STATUS "Looking for gLite API - NO")
  endif (HAVE_gLite)
endif (BUILD_GLITE_PLUGIN)

#
# LSF plug-in
#
if( BUILD_LSF_PLUGIN )
  find_package( LSF )
  message (STATUS "Looking for LSF API")
  if (LSF_FOUND)
     message (STATUS "Looking for LSF API - YES")
     message (STATUS "Build LSF plug-in")
     add_subdirectory ( plugins/LSFPlugin )
     message (STATUS "Build LSF plug-in - YES")
  else (LSF_FOUND)
     message (STATUS "Looking for LSF API - NO")
  endif (LSF_FOUND)
endif (BUILD_LSF_PLUGIN)

#
# PBS plug-in
#
if( BUILD_PBS_PLUGIN )
  message (STATUS "Looking for PBS API")
  find_package( PBS )
  if (PBS_FOUND)
     message (STATUS "Looking for PBS API - YES")
     message (STATUS "Build PBS plug-in")
     add_subdirectory ( plugins/PBSPlugin )
     message (STATUS "Build PBS plug-in - YES")
     if (BUILD_TESTS)
        add_subdirectory ( plugins/PBSPlugin/tests )
        message (STATUS "Build tests of pbs plug-in")
     endif (BUILD_TESTS)
  else (PBS_FOUND)
     message (STATUS "Looking for PBS API - NO") 
  endif (PBS_FOUND)
endif( BUILD_PBS_PLUGIN )

#
# Alien Plug-in
#
#find_package( Alien REQUIRED )
#if (ALIEN_FOUND)
#    add_subdirectory ( plugins/AlienPlugin )
#    message (STATUS "Build Alien plug-in - YES")
#else (ALIEN_FOUND)
#    message (STATUS "Build Alien plug-in - NO")
#endif (ALIEN_FOUND)


#
# Install Target
#
install(TARGETS pod-console DESTINATION bin)
