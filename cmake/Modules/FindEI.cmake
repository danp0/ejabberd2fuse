#
# Find erl interface includes and lib
#
# EI_INCLUDE_DIR    - erl interface include
# EI_LIB_DIR        - erl interface libraries directory
# ERL_INTERFACE_LIB - erl interface library
# EI_LIB            - ei library
# EI_FOUND          - erl interface found
#
IF (EI_INCLUDE_DIR)
  SET (EI_FIND_QUIETLY TRUE)
ENDIF (EI_INCLUDE_DIR)

FIND_PATH(EI_INCLUDE_DIR erl_interface.h
  /usr/lib/erlang/lib/erl_interface-3.7.5/include
  /usr/lib/erlang/usr/include
  /usr/lib64/erlang/usr/include)

FIND_PATH(EI_LIB_DIR liberl_interface.a
  /usr/lib/erlang/lib/erl_interface-3.7.5/lib
  /usr/lib/erlang/usr/lib
  /usr/lib64/erlang/usr/lib)

FIND_LIBRARY(ERL_INTERFACE_LIB
  NAMES erl_interface
  PATHS /usr/lib/erlang/lib/erl_interface-3.7.5/lib
        /usr/lib/erlang/usr/lib 
	/usr/lib64/erlang/lib)

FIND_LIBRARY(EI_LIB
  NAMES ei
  PATHS /usr/lib/erlang/lib/erl_interface-3.7.5/lib
        /usr/lib/erlang/usr/lib 
        /usr/lib64/erlang/lib)

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("EI" DEFAULT_MSG EI_INCLUDE_DIR EI_LIB_DIR ERL_INTERFACE_LIB EI_LIB)

mark_as_advanced (EI_INCLUDE_DIR EI_LIB_DIR ERL_INTERFACE_LIB EI_LIB)

