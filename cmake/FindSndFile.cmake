#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, December 2013
#
# ...but basically copied from the examples on the web, so it's not
# clear it's really copyright me.  This one is from
# http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries, with libXml2
# replaced by sndfile.
#

# - Try to find SndFile
# Once done this will define
#  SNDFILE_FOUND - System has SndFile
#  SNDFILE_INCLUDE_DIRS - The SndFile include directories
#  SNDFILE_LIBRARIES - The libraries needed to use SndFile
#  SNDFILE_DEFINITIONS - Compiler switches required for using SndFile

find_package(PkgConfig)
pkg_check_modules(PC_SNDFILE QUIET sndfile)
set(SNDFILE_DEFINITIONS ${PC_SNDFILE_CFLAGS_OTHER})

find_path(SNDFILE_INCLUDE_DIR sndfile.h
  HINTS ${PC_SNDFILE_INCLUDEDIR} ${PC_SNDFILE_INCLUDE_DIRS}
  PATH_SUFFIXES sndfile)

find_library(SNDFILE_LIBRARY NAMES sndfile
  HINTS ${PC_SNDFILE_LIBDIR} ${PC_SNDFILE_LIBRARY_DIRS})

set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SndFile DEFAULT_MSG
  SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)

mark_as_advanced(SNDFILE_INCLUDE_DIR SNDFILE_LIBRARY)
