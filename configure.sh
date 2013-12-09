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

# I like clang recently as the error messages are nicer than gcc
export CC=clang
export CXX=clang++

# Try for MKL; otherwise it'll find OpenBLAS or the like
export BLA_VENDOR=Intel10_64lp

# Download a test file for sndfile
if [ ! -e arctic_a0001.wav ]
then
    arctic=http://www.speech.cs.cmu.edu/cmu_arctic
    wget $arctic/cmu_us_bdl_arctic/wav/arctic_a0001.wav
    ln -s arctic_a0001.wav tests.wav
fi

cmake \
    -D CMAKE_BUILD_TYPE=debug \
    .
