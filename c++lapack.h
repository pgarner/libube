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

// This define is needed for lapacke.h to compile
#define HAVE_LAPACK_CONFIG_H
#include "lapacke.h"

/**
 * Templated LAPACK calls
 * Probably incomplete; just add as needed.
 * ...but lapacke.h is 16000 odd lines, so this could get really big.
 *
 * As for cblas, we could wrap the fortran calls instead.
 */
namespace lapack
{
    template<class T> long geev(long n, T* a, T* wr, T* wi, T* vl, T* vr);
    template<> long geev<float>(
        long n, float* a, float* wr, float* wi, float* vl, float* vr
    ) {
        return LAPACKE_sgeev(
            LAPACK_ROW_MAJOR,
            (vl ? 'V' : 'N'),
            (vr ? 'V' : 'N'),
            n, a, n, wr, wi,
            (vl ? vl : 0), n,
            (vr ? vr : 0), n
        );
    }
}

#endif // CXXLAPACK_H
