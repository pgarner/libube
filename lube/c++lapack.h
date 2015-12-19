/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, March 2015
 */

#ifndef CXXLAPACK_H
#define CXXLAPACK_H

/**
 * Templated LAPACK calls
 * Probably incomplete; just add as needed.
 * ...but clapack.h is 7000 odd lines, so this could get really big.
 */
namespace lapack
{
    template<class T> long gees(long n, T* a, T* wr, T* wi, T* vs=0);
    template<class T> long geev(long n, T* a, T* wr, T* wi, T* vl=0, T* vr=0);
}

#endif // CXXLAPACK_H
