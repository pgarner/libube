#!/bin/sh
#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
rm -f CMakeCache.txt CMakeFiles cmake_install.cmake
cmake \
    -D CMAKE_BUILD_TYPE=debug \
    .
