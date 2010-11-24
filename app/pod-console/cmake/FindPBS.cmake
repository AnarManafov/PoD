#************************************************************************
#
# FindPBS.cmake
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
# - Try to find PBS API libs and headers
# Once done this will define
#  
#  PBS_FOUND        - system has PBS development package
#  PBS_INCLUDE_DIR  - the include directory  
#  PBS_LIBRARIES    - Link these to use PBS API

# Parameters
# PBS_PREFIX - set this veriable to help the script to find PBS development files


IF (UNIX)

    FIND_PATH(PBS_INCLUDE_DIR pbs_ifl.h
      ${PBS_PREFIX}/include
      $ENV{PBS_PREFIX}/include
      /usr/local/include
      /usr/include
      /usr/local/include/torque
      /usr/include/torque
    )

    FIND_LIBRARY(PBS_TORQUE_LIB
      NAMES torque
      PATHS ${PBS_PREFIX}/lib
            $ENV{PBS_PREFIX}/lib
            /usr/local/lib
            /usr/lib
    )
    
    FIND_LIBRARY(PBS_PBS_LIB
      NAMES pbs
      PATHS ${PBS_PREFIX}/lib
            $ENV{PBS_PREFIX}/lib
            /usr/local/lib
            /usr/lib
    )
    
ENDIF (UNIX)

SET(PBS_FOUND "NO")

IF (PBS_TORQUE_LIB)
           
    IF(PBS_INCLUDE_DIR)
      # TODO: check that the bat library was also found
      SET(PBS_LIBRARIES ${PBS_TORQUE_LIB} )
      SET(PBS_FOUND "YES")
    ELSE(PBS_INCLUDE_DIR)
      SET(PBS_FOUND "NO")
    ENDIF(PBS_INCLUDE_DIR)
    
ENDIF (PBS_TORQUE_LIB)

IF (PBS_PBS_LIB)
           
    IF(PBS_INCLUDE_DIR)
      # TODO: check that the bat library was also found
      SET(PBS_LIBRARIES ${PBS_PBS_LIB} )
      SET(PBS_FOUND "YES")
    ELSE(PBS_INCLUDE_DIR)
      SET(PBS_FOUND "NO")
    ENDIF(PBS_INCLUDE_DIR)
    
ENDIF (PBS_PBS_LIB)


MARK_AS_ADVANCED(
  PBS_INCLUDE_DIR
  PBS_LIBRARIES
)

