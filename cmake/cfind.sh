#!/bin/zsh
#
# Copyright 2016 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, January 2016
#
function usage
cat <<EOF
Generates cmake FindXYZ.cmake files
See: https://cmake.org/Wiki/CMake:How_To_Find_Libraries
 Usage: cfind.sh <PackageName>
The output may need to edited for particular paths and names
EOF

# We need at least one argument
if [[ $# -lt 1 ]]
then
    usage
    exit 1
fi

# Assume we get the mixed case one; convert to upper and lower case
mixed=$1
upper=$1:u
lower=$1:l

# Dump the finder with appropriate substitutions
#
# pkgconfig also defines PC_XYX_VERSION, which can be used to set
# XYZ_VERSION_STRING.  However, if pkgconfig is not found, that won't get set.
cat <<EOF $output
#
# Written by cfind.sh
# Part of lube, https://github.com/pgarner/libube
# See also https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#
# The following should end up defined: 
#  ${upper}_FOUND          - System has ${mixed}
#  ${upper}_INCLUDE_DIR    - The ${mixed} include directories
#  ${upper}_LIBRARIES      - The libraries needed to use ${mixed}
#  ${upper}_DEFINITIONS    - Compiler switches required for using ${mixed}
#
find_package(PkgConfig)
pkg_check_modules(PC_${upper} QUIET ${lower})

set(${upper}_DEFINITIONS \${PC_${upper}_CFLAGS_OTHER})

find_path(
  ${upper}_INCLUDE_DIR ${lower}.h
  HINTS \${PC_${upper}_INCLUDEDIR} \${PC_${upper}_INCLUDE_DIRS}
  PATH_SUFFIXES ${lower}
)

find_library(
  ${upper}_LIBRARY ${lower}  # Without the lib- prefix
  HINTS \${PC_${upper}_LIBDIR} \${PC_${upper}_LIBRARY_DIRS}
)

set(${upper}_LIBRARIES \${${upper}_LIBRARY})  # Can add \${CMAKE_DL_LIBS}
set(${upper}_INCLUDE_DIRS \${${upper}_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  ${mixed} DEFAULT_MSG
  ${upper}_LIBRARY ${upper}_INCLUDE_DIR
)

mark_as_advanced(${upper}_INCLUDE_DIR ${upper}_LIBRARY)
EOF
