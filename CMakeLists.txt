#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
project(libvar)
cmake_minimum_required(VERSION 2.8)

# BLAS
find_package(BLAS REQUIRED)

# add_definitions(-DVARBOSE)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

set(HEADERS var.h)
set(SOURCES
  varfile.cpp
  varheap.cpp
  math.cpp
  string.cpp
  var.cpp
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(static-lib STATIC ${SOURCES})
set_target_properties(static-lib
  PROPERTIES OUTPUT_NAME   "var"
  PUBLIC_HEADER "${HEADERS}"
  )
target_link_libraries(static-lib "-ldl" "-lpthread" ${BLAS_LIBRARIES})

add_executable(tests tests.cpp)
target_link_libraries(tests static-lib)

add_executable(aspect aspect.cpp)
target_link_libraries(aspect static-lib)

# Shared dynamic libraries
add_library(txt-lib SHARED txtfile.cpp)
set_target_properties(txt-lib
  PROPERTIES OUTPUT_NAME   "txt"
  )
add_library(ini-lib SHARED inifile.cpp)
set_target_properties(ini-lib
  PROPERTIES OUTPUT_NAME   "ini"
  )
add_library(gnuplot-lib SHARED gnuplot.cpp)
set_target_properties(gnuplot-lib
  PROPERTIES OUTPUT_NAME   "gnuplot"
  )
add_library(ged-lib SHARED gedfile.cpp)
set_target_properties(ged-lib
  PROPERTIES OUTPUT_NAME   "ged"
  )

# Testing
enable_testing()
add_test(Tests tests one two three)
