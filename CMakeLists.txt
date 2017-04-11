cmake_minimum_required(VERSION 2.8)

project(ejabberd2fuse)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -Wall --pedantic -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(FUSE REQUIRED)
find_package(EI REQUIRED)

message(STATUS "${EI_INCLUDE_DIR}")
message(STATUS "${EI_LIB_DIR}")
message(STATUS "${ERL_INTERFACE_LIB}")
message(STATUS "${EI_LIB}")

include_directories(${FUSE_INCLUDE_DIR} ${EI_INCLUDE_DIR})
add_executable(ejabberd2fuse 
  ejabberd.c
  ejabberd2fuse.c
  main.c)
target_link_libraries(ejabberd2fuse pthread ${FUSE_LIBRARIES} ${ERL_INTERFACE_LIB} ${EI_LIB})

