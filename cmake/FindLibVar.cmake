#
# Copyright 2013 by Idiap Research Institute, http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2013
#

#
# Try to find LibVar
# Once done this will define
#  LIBVAR_FOUND          - System has LibVar
#  LIBVAR_INCLUDE_DIR    - The LibVar include directory
#  LIBVAR_LIBRARIES      - The libraries needed to use LibVar
#  LIBVAR_DEFINITIONS    - Compiler switches required for using LibVar
#  LIBVAR_VERSION_STRING - the version of LibVar found
#

find_package(PkgConfig)
pkg_check_modules(PC_LIBVAR QUIET libvar)

set(LIBVAR_DEFINITIONS ${PC_LIBVAR_CFLAGS_OTHER})
set(LIBVAR_VERSION_STRING ${PC_LIBVAR_VERSION})

find_path(
  LIBVAR_INCLUDE_DIR var.h
  HINTS ${PC_LIBVAR_INCLUDEDIR} ${PC_LIBVAR_INCLUDE_DIRS}
  PATH_SUFFIXES libvar
)

find_library(
  LIBVAR_LIBRARIES var
  HINTS ${PC_LIBVAR_LIBDIR} ${PC_LIBVAR_LIBRARY_DIRS}
)
list(APPEND LIBVAR_LIBRARIES ${CMAKE_DL_LIBS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibVar
  REQUIRED_VARS LIBVAR_LIBRARIES LIBVAR_INCLUDE_DIR
  VERSION_VAR LIBVAR_VERSION_STRING
)

mark_as_advanced(LIBVAR_INCLUDE_DIR LIBVAR_LIBRARIES)
