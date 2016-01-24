#!/bin/sh
#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
# This is for a $USER/local install.  Distributions should probably
# duplicate the necessary bits of this in the build file then call
# cmake directly.
#
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake

# I like clang recently as the error messages are nicer than gcc
# export CC=clang
# export CXX=clang++

# Try for MKL; otherwise it'll find OpenBLAS or the like
# export BLA_VENDOR=Intel10_64lp

# Build the static library?
export USE_STATIC=0

cmake \
    -D CMAKE_BUILD_TYPE=minsizerel \
    -D CMAKE_INSTALL_PREFIX=~/local \
    ..
