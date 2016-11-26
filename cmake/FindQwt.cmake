#
# Written by cfind.sh
# Part of lube, https://github.com/pgarner/libube
# See also https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#
# The following should end up defined: 
#  QWT_FOUND          - System has Qwt
#  QWT_INCLUDE_DIR    - The Qwt include directories
#  QWT_LIBRARIES      - The libraries needed to use Qwt
#  QWT_DEFINITIONS    - Compiler switches required for using Qwt
#
find_package(PkgConfig)
pkg_check_modules(PC_QWT QUIET qwt)

set(QWT_DEFINITIONS ${PC_QWT_CFLAGS_OTHER})

find_path(
  QWT_INCLUDE_DIR qwt.h
  HINTS ${PC_QWT_INCLUDEDIR} ${PC_QWT_INCLUDE_DIRS}
  PATH_SUFFIXES qwt
)

find_library(
  QWT_LIBRARY qwt  # Without the lib- prefix
  HINTS ${PC_QWT_LIBDIR} ${PC_QWT_LIBRARY_DIRS}
)

set(QWT_LIBRARIES ${QWT_LIBRARY})  # Can add ${CMAKE_DL_LIBS}
set(QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Qwt DEFAULT_MSG
  QWT_LIBRARY QWT_INCLUDE_DIR
)

mark_as_advanced(QWT_INCLUDE_DIR QWT_LIBRARY)
