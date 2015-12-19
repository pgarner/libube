/*
 * Copyright 2015 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2015
 */

#include <complex>

// From: http://www.netlib.org/clapack/f2c.h
#define VOID void
typedef long int integer;
typedef long int logical;
typedef long int ftnlen;
typedef float real;
typedef double doublereal;
typedef std::complex<float> complex;
typedef std::complex<double> doublecomplex;
typedef logical (*L_fp)(...);

// clapack is f2c'd lapack.  It's still fortran calling convention.
#include <clapack.h>
#include "c++lapack.h"

static char sV = 'V';
static char sN = 'N';

namespace lapack
{
    template<> long gees<float>(
        long n, float* a, float* wr, float* wi, float* vs
    ) {
        long sdim;
        long info;
        long lwork = std::max(1L,n*3);
        float work[lwork];
        sgees_(
            (vs ? &sV : &sN),
            &sN, 0,
            &n, a, &n, &sdim, wr, wi,
            (vs ? vs : 0), &n,
            work, &lwork, 0, &info
        );
        return info;
    }

    template<> long geev<float>(
        long n, float* a, float* wr, float* wi, float* vl, float* vr
    ) {
        long info;
        long lwork = std::max(1L,n*4);
        float work[lwork];
        sgeev_(
            (vr ? &sV : &sN),
            (vl ? &sV : &sN),
            &n, a, &n, wr, wi,
            (vr ? vr : 0), &n,
            (vl ? vl : 0), &n,
            work, &lwork, &info
        );
        return info;
    }

    template<> long geev<double>(
        long n, double* a, double* wr, double* wi, double* vl, double* vr
    ) {
        long info;
        long lwork = std::max(1L,n*4);
        double work[lwork];
        dgeev_(
            (vr ? &sV : &sN),
            (vl ? &sV : &sN),
            &n, a, &n, wr, wi,
            (vr ? vr : 0), &n,
            (vl ? vl : 0), &n,
            work, &lwork, &info
        );
        return info;
    }
}
