#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
cmake_minimum_required(VERSION 3.7)

# Package information
project(libube VERSION 1.0.2)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kissfft")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# External deps
include(KissFFT)

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
