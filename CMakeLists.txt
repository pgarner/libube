#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
cmake_minimum_required(VERSION 2.8)

# Package information
project(libvar)
set(VERSION 0.1)

# Package stuff
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(BLAS REQUIRED)
find_package(Boost COMPONENTS regex REQUIRED)

# add_definitions(-DVARBOSE)
set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

set(HEADERS var.h var)
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
target_link_libraries(static-lib
  "-ldl" "-lpthread" ${BLAS_LIBRARIES} ${Boost_LIBRARIES}
  )
set(INSTALL_TARGETS static-lib)

add_executable(tests tests.cpp)
target_link_libraries(tests static-lib)

add_executable(aspect aspect.cpp)
target_link_libraries(aspect static-lib)

# Shared dynamic libraries
add_library(txt-lib SHARED txtfile.cpp)
set_target_properties(txt-lib
  PROPERTIES OUTPUT_NAME   "txt"
  )
list(APPEND INSTALL_TARGETS txt-lib)

add_library(ini-lib SHARED inifile.cpp)
set_target_properties(ini-lib
  PROPERTIES OUTPUT_NAME   "ini"
  )
list(APPEND INSTALL_TARGETS ini-lib)

add_library(gnuplot-lib SHARED gnuplot.cpp)
set_target_properties(gnuplot-lib
  PROPERTIES OUTPUT_NAME   "gnuplot"
  )
list(APPEND INSTALL_TARGETS gnuplot-lib)

add_library(ged-lib SHARED gedfile.cpp)
set_target_properties(ged-lib
  PROPERTIES OUTPUT_NAME   "ged"
  )
list(APPEND INSTALL_TARGETS ged-lib)

find_package(SndFile)
if (SNDFILE_FOUND)
  include_directories(${SNDFILE_INCLUDE_DIR})
  add_library(snd-lib SHARED sndfile.cpp)
  target_link_libraries(snd-lib ${SNDFILE_LIBRARIES})
  set_target_properties(snd-lib
    PROPERTIES OUTPUT_NAME   "snd"
    )
  list(APPEND INSTALL_TARGETS snd-lib)
endif (SNDFILE_FOUND)

find_package(EXPAT)
if (EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIR})
  add_library(xml-lib SHARED xmlfile.cpp)
  target_link_libraries(xml-lib ${EXPAT_LIBRARIES})
  set_target_properties(xml-lib
    PROPERTIES OUTPUT_NAME   "xml"
    )
  list(APPEND INSTALL_TARGETS xml-lib)
endif (EXPAT_FOUND)

# Testing
enable_testing()
add_test(
  NAME libvar
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/tests.cmake
  )

install(
  TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/libvar
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  )

# pkgconfig install lines
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(EXEC_PREFIX "\${prefix}")
set(LIBDIR "\${exec_prefix}/lib")
set(INCLUDEDIR "\${prefix}/include")
configure_file(libvar.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libvar.pc @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libvar.pc
  DESTINATION lib/pkgconfig
  )
