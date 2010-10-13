#************************************************************************
#
# FindOGE.cmake
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
# - Try to find SGE/OGE DRMAA API libs and headers
# Once done this will define
#  
#  OGE_FOUND        - system has OGE development package
#  OGE_INCLUDE_DIR  - the include directory  
#  OGE_LIBRARIES    - Link these to use OGE API

# The following OGE environment variables could help and should be defined:
# $SGE_ROOT, $SGE_ARCH

# Parameters
# OGE_PREFIX - set this veriable to help the script to find OGE development files


IF (UNIX)

    FIND_PATH(OGE_INCLUDE_DIR drmaa.h
      ${OGE_PREFIX}/include
      $ENV{OGE_PREFIX}/include
      $ENV{SGE_ROOT}/include
      /usr/local/include
      /usr/include
    )

    FIND_LIBRARY(OGE_DRMAA_LIB
      NAMES drmaa
      PATHS ${OGE_PREFIX}/lib
            ${OGE_PREFIX}/lib/$ENV{SGE_ARCH} 
            $ENV{OGE_PREFIX}/lib
            $ENV{OGE_PREFIX}/lib/$ENV{SGE_ARCH}
            $ENV{SGE_ROOT}/lib/$ENV{SGE_ARCH}
            /usr/local/lib
            /usr/lib
    )
    
ENDIF (UNIX)

SET(OGE_FOUND "NO")

IF (OGE_DRMAA_LIB)
           
    IF(OGE_INCLUDE_DIR)
      SET(OGE_LIBRARIES ${OGE_DRMAA_LIB} )
      SET(OGE_FOUND "YES")
    ELSE(OGE_INCLUDE_DIR)
      SET(OGE_FOUND "NO")
    ENDIF(OGE_INCLUDE_DIR)
    
ENDIF (OGE_TORQUE_LIB)


MARK_AS_ADVANCED(
  OGE_INCLUDE_DIR
  OGE_LIBRARIES
)

