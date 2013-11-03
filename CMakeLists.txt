project(var)
cmake_minimum_required(VERSION 2.8)

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
target_link_libraries(static-lib "-ldl")

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
