#!/bin/sh
#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake
mklvars=/opt/intel/mkl/bin/mklvars.sh
if [ -e $mkvars ]
then
    . $mklvars intel64
fi
cmake \
    -D CMAKE_BUILD_TYPE=debug \
    .
