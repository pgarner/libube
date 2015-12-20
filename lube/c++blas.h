/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, January 2015
 */

#ifndef CXXBLAS_H
#define CXXBLAS_H

/**
 * Templated BLAS calls
 * Probably incomplete; just add as needed.
 */
namespace blas
{
    template<class T> void copy(long iN, T* iX, T* iY);
    template<class T> void axpy(long iN, T iAlpha, T* iX, T* ioY);
    template<class T> void scal(long iN, T iAlpha, T* ioX);
    template<class T> T dot(long iN, T* iX, T* iY);
    template<class T> long iamax(long iN, T* iX);
    template<class T> T asum(long iN, T* iX);
    template<class T> void tbmv(long iN, long iK, T* iA, T* ioX);
    template<class T> void sbmv(
        long iN, long iK, T iAlpha, T* iA, T* iX, T iBeta, T* ioY
    );
    template<class T> void gemm(
        long iM, long iN, long iK, T iAlpha, T* iA, T* iB, T iBeta, T* ioC
    );
}

#endif // CXXBLAS_H
