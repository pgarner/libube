#
# Copyright 2015 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, June 2015
#

find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)
find_package(Boost COMPONENTS
  regex REQUIRED
  filesystem REQUIRED
  system REQUIRED
  )

include_directories(
  ${BLAS_INCLUDE_DIRS}
  ${LAPACK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

set(HEADERS
  var.h
  ind.h
  func.h
  math.h
  string.h
  regex.h
  path.h
  config.h
  graph.h
  c++blas.h
  c++lapack.h
  data.h
  module.h
  curl.h
  dft.h
  )

set(SOURCES
  var.cpp
  heap.cpp
  module.cpp
  math.cpp
  func.cpp
  string.cpp
  config.cpp
  clapack.cpp
  json.cpp
  utf8.cpp
  stream.cpp
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
  list(APPEND SOURCES
    dft-kiss.cpp
    transpose-adhoc.cpp
    ${KISSFFT_DIR}/tools/kiss_fftr.c
    ${KISSFFT_DIR}/kiss_fft.c
    )
  include_directories(${KISSFFT_DIR} ${KISSFFT_DIR}/tools)
  message(STATUS "MKL seems not available; using Kiss FFT & ad-hoc transpose")
endif (USE_MKL)

# MacOS Accelerate (and perhaps others) uses f2c return conventions
set(USE_F2C_BLAS ${BLAS_Accelerate_LIBRARY})
if (USE_F2C_BLAS)
  add_definitions(-DHAVE_F2C_BLAS)
  message(STATUS "The BLAS library has f2c return conventions")
endif (USE_F2C_BLAS)

# Threads are for BLAS; not sure why it's not included in BLAS_LIBRARIES
set(TARGET_LIBS
  -ldl
  ${BLAS_LIBRARIES}
  ${LAPACK_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  )

# Shared library
#
# The main point here is that the shared library can have an RPATH
# attribute that locates the modules.  Anything linking with this
# effectively inherits that RPATH, so it needn't do anything special
# about modules.
#
# Putting both the local and installed RPATH in at the outset allows building
# in the source tree (it worked until early 2018, why?) and allows other source
# trees to provide modules in a directory called lube.
add_library(lube-shared SHARED ${SOURCES})
set_target_properties(lube-shared
  PROPERTIES OUTPUT_NAME "ube"
  PUBLIC_HEADER "${HEADERS}"
  INSTALL_RPATH "lube:${CMAKE_INSTALL_PREFIX}/lib/lube"
  BUILD_WITH_INSTALL_RPATH "TRUE"
  )
target_link_libraries(lube-shared ${TARGET_LIBS})
list(APPEND LIBUBE_TARGETS lube-shared)

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
    PROPERTIES OUTPUT_NAME "ube"
    )
  target_link_libraries(static-lib ${TARGET_LIBS})
  list(APPEND LIBUBE_TARGETS static-lib)
endif (USE_STATIC)

# Shared dynamic libraries
#
# CMake has a special MODULE designation for these, which is handy,
# but doesn't distinguish the install location from SHARED.  Hence,
# different target list so they can be installed to a different
# directory.

add_library(path-lib MODULE path.cpp)
target_link_libraries(path-lib lube-shared)
set_target_properties(path-lib
  PROPERTIES OUTPUT_NAME "path"
  )
list(APPEND MODULE_TARGETS path-lib)

add_library(txt-lib MODULE txtfile.cpp)
target_link_libraries(txt-lib lube-shared)
set_target_properties(txt-lib
  PROPERTIES OUTPUT_NAME "txt"
  )
list(APPEND MODULE_TARGETS txt-lib)

add_library(ini-lib MODULE inifile.cpp)
target_link_libraries(ini-lib lube-shared)
set_target_properties(ini-lib
  PROPERTIES OUTPUT_NAME "ini"
  )
list(APPEND MODULE_TARGETS ini-lib)

add_library(gnuplot-lib MODULE gnuplot.cpp)
target_link_libraries(gnuplot-lib lube-shared)
set_target_properties(gnuplot-lib
  PROPERTIES OUTPUT_NAME "gnuplot"
  )
list(APPEND MODULE_TARGETS gnuplot-lib)

add_library(ged-lib MODULE gedfile.cpp)
target_link_libraries(ged-lib lube-shared)
set_target_properties(ged-lib
  PROPERTIES OUTPUT_NAME "ged"
  )
list(APPEND MODULE_TARGETS ged-lib)

find_package(SndFile)
if (SNDFILE_FOUND)
  include_directories(${SNDFILE_INCLUDE_DIRS})
  add_library(snd-lib MODULE sndfile.cpp)
  target_link_libraries(snd-lib lube-shared ${SNDFILE_LIBRARIES})
  set_target_properties(snd-lib
    PROPERTIES OUTPUT_NAME "snd"
    )
  list(APPEND MODULE_TARGETS snd-lib)
endif (SNDFILE_FOUND)

find_package(EXPAT)
if (EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIRS})
  add_library(xml-lib MODULE xmlfile.cpp)
  target_link_libraries(xml-lib lube-shared ${EXPAT_LIBRARIES})
  set_target_properties(xml-lib
    PROPERTIES OUTPUT_NAME "xml"
    )
  list(APPEND MODULE_TARGETS xml-lib)
endif (EXPAT_FOUND)

add_library(graph-lib MODULE graph.cpp)
target_link_libraries(graph-lib lube-shared)
set_target_properties(graph-lib
  PROPERTIES OUTPUT_NAME "graph"
  )
list(APPEND MODULE_TARGETS graph-lib)

find_package(CURL)
if (CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIRS})
  add_library(curl-lib MODULE curl.cpp)
  target_link_libraries(curl-lib lube-shared ${CURL_LIBRARIES})
  set_target_properties(curl-lib
    PROPERTIES OUTPUT_NAME "curl"
    )
  list(APPEND MODULE_TARGETS curl-lib)
endif (CURL_FOUND)

find_package(Qwt)
if (QWT_FOUND)
  find_package(Qt5Widgets REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QWT_INCLUDE_DIRS})
  add_definitions(${Qt5Widgets_DEFINITIONS} ${QWT_DEFINITIONS})
  # qt5_wrap_cpp(QWT_HEADERS_MOC qwtplot.h)
  add_library(qwt-lib MODULE qwt.cpp ${QWT_HEADERS_MOC})
  target_link_libraries(qwt-lib
    lube-shared
    ${Qt5Widgets_LIBRARIES}
    ${QWT_LIBRARIES}
    )
  set_target_properties(qwt-lib
    PROPERTIES OUTPUT_NAME "qwt"
    )
  list(APPEND MODULE_TARGETS qwt-lib)
endif (QWT_FOUND)

# Normal install path for the library
install(
  TARGETS ${LIBUBE_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/lube
  )

# Slightly different location for the modules
install(
  TARGETS ${MODULE_TARGETS}
  LIBRARY DESTINATION lib/lube
  )
