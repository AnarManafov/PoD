#************************************************************************
#
# FindLSF.cmake
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2008-12-30
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2008 GSI GridTeam. All rights reserved.
#*************************************************************************
# - Try to find LSF
# Once done this will define
#  
#  LSF_FOUND        - system has LSF development package
#  LSF_INCLUDE_DIR  - the LSG include directory  
#  LSF_LIBRARIES    - Link these to use LSF

# Parameters
# LSF_PREFIX - set this veriable to help this script to find LSF development files


IF (UNIX)

    FIND_PATH(LSF_INCLUDE_DIR lsf/lsf.h
      ${LSF_PREFIX}/include
      /LSF/lsf
    )

    FIND_LIBRARY(LSF_LSF_LIB
      NAMES lsf
      PATHS ${LSF_PREFIX}/lib
            /LSF/lsf/lib
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
