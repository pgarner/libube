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
project(libvar)
set(VERSION 0.1)
# enable_language(Fortran)

# Package stuff
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(BLAS REQUIRED)
find_package(Boost COMPONENTS
  regex REQUIRED
  filesystem REQUIRED
  system REQUIRED
  )

# add_definitions(-DVARBOSE)
# Compile with "-std=c++11 -stdlib=libc++ -lc++abi" to use libc++
set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++14")

set(HEADERS var.h ind.h lv.h path.h config.h)
set(SOURCES
  var.cpp
  heap.cpp
  file.cpp
  math.cpp
  string.cpp
  config.cpp
  )

# If we have MKL then we should use the DFT and matrix transpose in MKL (in
# addition to the BLAS stuff in there).  If not, BLAS is covered by OpenBLAS
# and the like, but we need a fallback for the DFT and transpose.  Right now,
# that's Kiss FFT and an ad-hoc matrix transpose.  These are not optimal!
# ...not least because Kiss FFT is only single precision.
set(USE_MKL ${BLAS_mkl_core_LIBRARY})
if (USE_MKL)
  list(APPEND SOURCES
    dft-mkl.cpp
    transpose-mkl.cpp
    )
  add_definitions(-DHAVE_MKL)
  message(STATUS "MKL BLAS seems available; using it for DFT & transpose")
else (USE_MKL)
  set(KISSDIR $ENV{KISSDIR}
    CACHE FILEPATH "Path to Kiss FFT directory"
    )
  list(APPEND SOURCES
    dft-kiss.cpp
    transpose-adhoc.cpp
    ${KISSDIR}/tools/kiss_fftr.c
    ${KISSDIR}/kiss_fft.c
    )
  include_directories(${KISSDIR} ${KISSDIR}/tools)
  message(STATUS "MKL seems not available; using Kiss FFT & ad-hoc transpose")
endif (USE_MKL)

# Threads are for BLAS; not sure why it's not included in BLAS_LIBRARIES
set(TARGET_LIBS
  -ldl
  ${BLAS_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  )

# Shared library
#
# The main point here is that the shared library can have an RPATH
# attribute that locates the modules.  Anything linking with this
# effectively inherits that RPATH, so it needn't do anything special
# about modules.
add_library(shared-lib SHARED ${SOURCES})
set_target_properties(shared-lib
  PROPERTIES OUTPUT_NAME "var"
  PUBLIC_HEADER "${HEADERS}"
  INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/libvar"
  )
target_link_libraries(shared-lib ${TARGET_LIBS})
list(APPEND LIBVAR_TARGETS shared-lib)

# Static library
#
# The idea behind the static library is that BLAS can run faster when
# statically linked.  However, if used, the resulting executables will
# have to set an explicit RPATH to find modules.
set(USE_STATIC $ENV{USE_STATIC}
  CACHE BOOLEAN "Whether to build static library"
  )
if (USE_STATIC)
  add_library(static-lib STATIC ${SOURCES})
  set_target_properties(static-lib
    PROPERTIES OUTPUT_NAME "var"
    )
  target_link_libraries(static-lib ${TARGET_LIBS})
  list(APPEND LIBVAR_TARGETS static-lib)
endif (USE_STATIC)

# Shared dynamic libraries
#
# CMake has a special MODULE designation for these, which is handy,
# but doesn't distinguish the install location from SHARED.  Hence,
# different target list so they can be installed to a different
# directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(path-lib MODULE path.cpp)
target_link_libraries(path-lib ${Boost_LIBRARIES})
set_target_properties(path-lib
  PROPERTIES OUTPUT_NAME "path"
  )
list(APPEND MODULE_TARGETS path-lib)

add_library(txt-lib MODULE txtfile.cpp)
set_target_properties(txt-lib
  PROPERTIES OUTPUT_NAME "txt"
  )
list(APPEND MODULE_TARGETS txt-lib)

add_library(ini-lib MODULE inifile.cpp)
set_target_properties(ini-lib
  PROPERTIES OUTPUT_NAME "ini"
  )
list(APPEND MODULE_TARGETS ini-lib)

add_library(gnuplot-lib MODULE gnuplot.cpp)
set_target_properties(gnuplot-lib
  PROPERTIES OUTPUT_NAME "gnuplot"
  )
list(APPEND MODULE_TARGETS gnuplot-lib)

add_library(ged-lib MODULE gedfile.cpp)
set_target_properties(ged-lib
  PROPERTIES OUTPUT_NAME "ged"
  )
list(APPEND MODULE_TARGETS ged-lib)

find_package(SndFile)
if (SNDFILE_FOUND)
  include_directories(${SNDFILE_INCLUDE_DIR})
  add_library(snd-lib MODULE sndfile.cpp)
  target_link_libraries(snd-lib ${SNDFILE_LIBRARIES})
  set_target_properties(snd-lib
    PROPERTIES OUTPUT_NAME "snd"
    )
  list(APPEND MODULE_TARGETS snd-lib)
endif (SNDFILE_FOUND)

find_package(EXPAT)
if (EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIR})
  add_library(xml-lib MODULE xmlfile.cpp)
  target_link_libraries(xml-lib ${EXPAT_LIBRARIES})
  set_target_properties(xml-lib
    PROPERTIES OUTPUT_NAME "xml"
    )
  list(APPEND MODULE_TARGETS xml-lib)
endif (EXPAT_FOUND)

# Testing
#
# The tests get built here, but they are run from a different cmake
# file.  This lets us compare output with a reference output rather
# than just test that something runs.
enable_testing()

add_executable(test-ind test-ind.cpp)
add_test(
  NAME ind
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-ind.cmake
  )

add_executable(test-var test-var.cpp)
target_link_libraries(test-var shared-lib)
add_test(
  NAME var
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-var.cmake
  )

add_executable(test-file test-file.cpp)
target_link_libraries(test-file shared-lib)
add_test(
  NAME file
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-file.cmake
  )

add_executable(test-math test-math.cpp)
target_link_libraries(test-math shared-lib)
add_test(
  NAME math
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-math.cmake
  )

add_executable(test-path test-path.cpp)
target_link_libraries(test-path shared-lib)
add_test(
  NAME path
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-path.cmake
  )

add_executable(test-lapack test-lapack.cpp)
target_link_libraries(test-lapack shared-lib)
add_test(
  NAME lapack
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-lapack.cmake
  )

add_executable(test-config test-config.cpp)
target_link_libraries(test-config shared-lib)
add_test(
  NAME config
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/test-config.cmake
  )

# A test file specifically for exceptions
add_executable(throw throw.cpp)
target_link_libraries(throw shared-lib)

# Normal install path for the library
install(
  TARGETS ${LIBVAR_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  )

# Slightly different location for the modules
install(
  TARGETS ${MODULE_TARGETS}
  LIBRARY DESTINATION lib/libvar
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
