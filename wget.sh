#!/bin/sh
#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#

# Download a test file for sndfile
if [ ! -e arctic_a0001.wav ]
then
    arctic=http://www.speech.cs.cmu.edu/cmu_arctic
    wget $arctic/cmu_us_bdl_arctic/wav/arctic_a0001.wav
    ln -s ../arctic_a0001.wav test/test.wav
fi

# Download Kiss FFT (whether or not it's needed)
if [ ! -e $KISSDIR ]
then
    kisssrc=http://sourceforge.net/projects/kissfft/files/kissfft/v1_3_0
    wget $kisssrc/$KISSDIR.tar.gz
    tar zxf $KISSDIR.tar.gz
fi

# Download the reference clapack header.  This saves headaches with it being
# hidden in other optimised libraries (mkl.h ...)
if [ ! -e clapack.h ]
then
    wget http://www.netlib.org/clapack/clapack.h
fi
