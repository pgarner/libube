#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2013
#
# ...but basically copied from the examples on the web, so it's not
# clear it's really copyright me.  This one is basically from
# http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries, with libXml2
# replaced by sndfile.
#

#
# Try to find SndFile
# Once done this will define
#  SNDFILE_FOUND          - System has SndFile
#  SNDFILE_INCLUDE_DIR    - The SndFile include directories
#  SNDFILE_LIBRARIES      - The libraries needed to use SndFile
#  SNDFILE_DEFINITIONS    - Compiler switches required for using SndFile
#  SNDFILE_VERSION_STRING - the version of SndFile found
#

find_package(PkgConfig)
pkg_check_modules(PC_SNDFILE QUIET sndfile)

set(SNDFILE_DEFINITIONS ${PC_SNDFILE_CFLAGS_OTHER})
set(SNDFILE_VERSION_STRING ${PC_SNDFILE_VERSION})

find_path(
  SNDFILE_INCLUDE_DIR sndfile.h
  HINTS ${PC_SNDFILE_INCLUDEDIR} ${PC_SNDFILE_INCLUDE_DIRS}
  PATH_SUFFIXES sndfile
)

find_library(
  SNDFILE_LIBRARIES NAMES sndfile
  HINTS ${PC_SNDFILE_LIBDIR} ${PC_SNDFILE_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  SndFile
  REQUIRED_VARS SNDFILE_LIBRARIES SNDFILE_INCLUDE_DIR
  VERSION_VAR SNDFILE_VERSION_STRING
)

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARIES)
