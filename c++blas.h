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

#include <cblas.h>

/**
 * Templated BLAS calls
 * Probably incomplete; just add as needed.
 */
namespace blas
{
    template<>
    void copy<float>(long iSize, float* iX, float* iY) {
        cblas_scopy(iSize, iX, 1, iY, 1);
    }
    template<>
    void copy<double>(long iSize, double* iX, double* iY) {
        cblas_dcopy(iSize, iX, 1, iY, 1);
    }

    template<>
    void axpy<float>(long iSize, float iAlpha, float* iX, float* iY) {
        cblas_saxpy(iSize, iAlpha, iX, 1, iY, 1);
    }
    template<>
    void axpy<double>(long iSize, double iAlpha, double* iX, double* iY) {
        cblas_daxpy(iSize, iAlpha, iX, 1, iY, 1);
    }

    template<>
    void scal<float>(long iSize, float iAlpha, float* ioX) {
        cblas_sscal(iSize, iAlpha, ioX, 1);
    }
    template<>
    void scal<double>(long iSize, double iAlpha, double* ioX) {
        cblas_dscal(iSize, iAlpha, ioX, 1);
    }

    template<>
    float dot<float>(long iSize, float* iX, float* iY) {
        return cblas_sdot(iSize, iX, 1, iY, 1);
    }
    template<>
    double dot<double>(long iSize, double* iX, double* iY) {
        return cblas_ddot(iSize, iX, 1, iY, 1);
    }

    template<>
    long iamax<float>(long iSize, float* iX) {
        return cblas_isamax(iSize, iX, 1);
    }
    template<>
    long iamax<double>(long iSize, double* iX) {
        return cblas_idamax(iSize, iX, 1);
    }
}

#endif // CXXBLAS_H
