#
# Written by cfind.sh
# Part of lube, https://github.com/pgarner/libube
# See also https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#
# The following should end up defined: 
#  LIBUBE_FOUND          - System has LibUBE
#  LIBUBE_INCLUDE_DIR    - The LibUBE include directories
#  LIBUBE_LIBRARIES      - The libraries needed to use LibUBE
#  LIBUBE_DEFINITIONS    - Compiler switches required for using LibUBE
#
find_package(PkgConfig)
pkg_check_modules(PC_LIBUBE QUIET libube)

set(LIBUBE_DEFINITIONS ${PC_LIBUBE_CFLAGS_OTHER})

find_path(
  LIBUBE_INCLUDE_DIR lube.h
  HINTS ${PC_LIBUBE_INCLUDEDIR} ${PC_LIBUBE_INCLUDE_DIRS}
  PATH_SUFFIXES libube
)

find_library(
  LIBUBE_LIBRARY ube  # Without the lib- prefix
  HINTS ${PC_LIBUBE_LIBDIR} ${PC_LIBUBE_LIBRARY_DIRS}
)

set(LIBUBE_LIBRARIES ${LIBUBE_LIBRARY} ${CMAKE_DL_LIBS})
set(LIBUBE_INCLUDE_DIRS ${LIBUBE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibUBE DEFAULT_MSG
  LIBUBE_LIBRARY LIBUBE_INCLUDE_DIR
)

mark_as_advanced(LIBUBE_INCLUDE_DIR LIBUBE_LIBRARY)
