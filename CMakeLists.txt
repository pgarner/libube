#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
cmake_minimum_required(VERSION 3.0)

# Package information
project(libube)
set(VERSION 0.1)
# enable_language(Fortran)

# add_definitions(-DVARBOSE)
# Compile with "-std=c++11 -stdlib=libc++ -lc++abi" to use libc++
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kissfft")
set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++14")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# External deps
include(KissFFT)

enable_testing()

add_subdirectory(lube)
add_subdirectory(test)

# A test file specifically for exceptions
add_executable(throw throw.cpp)
target_link_libraries(throw lube-shared)

# The header "lube.h" is in the system include directory
install(
  FILES "lube.h"
  DESTINATION include
  )

# pkgconfig install lines
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(EXEC_PREFIX "\${prefix}")
set(LIBDIR "\${exec_prefix}/lib")
set(INCLUDEDIR "\${prefix}/include")
configure_file(libube.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libube.pc @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libube.pc
  DESTINATION lib/pkgconfig
  )
