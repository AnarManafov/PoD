#************************************************************************
#
# FindAlien.cmake
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2009-06-30
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2009 GSI GridTeam. All rights reserved.
#*************************************************************************
# - Try to find Alien
# Once done this will define
#  
#  ALIEN_FOUND        - system has Alien development package
#  ALIEN_INCLUDE_DIR  - the Alien include directory  
#  ALIEN_LIBRARIES    - Link these to use Alien

# Parameters
# ALIEN_PREFIX - set this veriable to help this script to find Alien development files


IF (UNIX)

    FIND_PATH(ALIEN_INCLUDE_DIR gapiUI.h
      ${ALIEN_PREFIX}/api/include
      /opt/alien/api/include
      $ENV{ALIEN}/api/include
      $ENV{ALIEN_ROOT}/api/include
    )

    FIND_LIBRARY(ALIEN_GAPIUI_LIB
      NAMES gapiUI
      PATHS ${ALIEN_PREFIX}/api/lib
            /opt/alien/api/lib
	    $ENV{ALIEN}/api/lib
            $ENV{ALIEN_ROOT}/api/lib
    )
    
    FIND_LIBRARY(LSF_BAT_LIB
      NAMES bat
      PATHS ${LSF_PREFIX}/lib
            /LSF/lsf/lib
    )
    
ENDIF (UNIX)

SET(LSF_FOUND "NO")
IF (LSF_LSF_LIB)
           
    IF(LSF_INCLUDE_DIR)
      # TODO: check that the bat library was also found
      SET( LSF_LIBRARIES ${LSF_LSF_LIB} ${LSF_BAT_LIB} )
      SET(LSF_FOUND "YES")
    ELSE(LSF_INCLUDE_DIR)
      SET(LSF_FOUND "NO")
    ENDIF(LSF_INCLUDE_DIR)
    
ENDIF (LSF_LSF_LIB)

MARK_AS_ADVANCED(
  LSF_INCLUDE_DIR
  LSF_LIB_DIR
)
