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

# This is only the cblas header, which defines the interface.  The
# implementation should be some optimised library like MKL or OpenBLAS
if [[ ! -e cblas.h ]]
then
    wget http://www.netlib.org/blas/blast-forum/cblas.tgz
    tar zxvf cblas.tgz CBLAS/include/cblas.h
    ln -s CBLAS/include/cblas.h
fi

cmake \
    -D CMAKE_BUILD_TYPE=debug \
    .
