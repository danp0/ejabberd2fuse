#
# Find FUSE includes and lib
# 
# FUSE_INCLUDE_DIR  - FUSE includes
# FUSE_LIBRARIES    - FUSE libraries
# FUSE_FOUND        - FUSE found
#
IF (FUSE_INCLUDE_DIR)
  SET (FUSE_FIND_QUIETLY TRUE)
ENDIF (FUSE_INCLUDE_DIR)

FIND_PATH(FUSE_INCLUDE_DIR fuse.h
  /usr/include
  /usr/local/include)

FIND_LIBRARY(FUSE_LIBRARIES
  NAMES fuse
  PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib)

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("FUSE" DEFAULT_MSG FUSE_INCLUDE_DIR FUSE_LIBRARIES)

mark_as_advanced (FUSE_INCLUDE_DIR FUSE_LIBRARIES)
