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
export CC=clang
export CXX=clang++

# Try for MKL; otherwise it'll find OpenBLAS or the like
export BLA_VENDOR=Intel10_64lp

# Download a test file for sndfile
if [ ! -e arctic_a0001.wav ]
then
    arctic=http://www.speech.cs.cmu.edu/cmu_arctic
    wget $arctic/cmu_us_bdl_arctic/wav/arctic_a0001.wav
    ln -s arctic_a0001.wav test.wav
fi

# Download Kiss FFT (whether or not it's needed)
export KISSDIR=kiss_fft130
if [ ! -e $KISSDIR ]
then
    kisssrc=http://sourceforge.net/projects/kissfft/files/kissfft/v1_3_0
    wget $kisssrc/$KISSDIR.tar.gz
    tar zxf $KISSDIR.tar.gz
fi

# Download the reference cblas header.  This saves headaches with it being
# hidden in other optimised libraries (mkl.h ...)
if [ ! -e cblas.h ]
then
    wget http://www.netlib.org/blas/blast-forum/cblas.tgz
    tar zxf cblas.tgz
    cp CBLAS/include/cblas.h .
fi

# Build the static library?
export USE_STATIC=0

cmake \
    -D CMAKE_BUILD_TYPE=minsizerel \
    -D CMAKE_INSTALL_PREFIX=~/local \
    .
