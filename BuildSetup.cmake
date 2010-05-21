#
# to build PoD issue the following commands:
#  1) mkdir build
#  2) cd build
#  3) cmake -C ../BuildSetup.cmake ..
#  4) make install
#


#
# BUILD TYPE
#
# set cmake build type, default value is: RelWithDebInfo
# possible options are: None Debug Release RelWithDebInfo MinSizeRel
set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE )

#
# GSI specific build
#
#set( GSI_BUILD ON CACHE BOOL "Build a GSI specific version" FORCE )

# gLite plug-in
#set( BUILD_GLITE_PLUGIN OFF CACHE BOOL "Build source code documentation" FORCE )

# LSF plug-in
#set( BUILD_LSF_PLUGIN OFF CACHE BOOL "Build source code documentation" FORCE )
#set( LSF_PREFIX "/home/anar/LSF" CACHE STRING "LSF prefix" FORCE)

# PBS plug-in
#set( BUILD_PBS_PLUGIN OFF CACHE BOOL "Build source code documentation" FORCE )
