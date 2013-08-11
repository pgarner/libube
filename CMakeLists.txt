project(var)
cmake_minimum_required(VERSION 2.8)

# add_definitions(-DVARBOSE)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

set(HEADERS var.h)
set(SOURCES
  var.cpp
  math.cpp
  string.cpp
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(static-lib STATIC ${SOURCES})
set_target_properties(static-lib
  PROPERTIES OUTPUT_NAME   "var"
  PUBLIC_HEADER "${HEADERS}"
  )

add_executable(tests tests.cpp)
target_link_libraries(tests static-lib)
