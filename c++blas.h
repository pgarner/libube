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
#include <complex>

#define CFLOAT std::complex<float>
#define CDOUBLE std::complex<double>

/**
 * Templated BLAS calls
 * Probably incomplete; just add as needed.
 */
namespace blas
{
    template<class T> void copy(long iSize, T* iX, T* iY);
    template<class T> void axpy(long iSize, T iAlpha, T* iX, T* iY);
    template<class T> void scal(long iSize, T iAlpha, T* ioX);
    template<class T> T dot(long iSize, T* iX, T* iY);
    template<class T> long iamax(long iSize, T* iX);
    template<class T> T asum(long iSize, T* iX);
    template<class T> void tbmv(long iSize, long iK, T* iA, T* iX);
    template<class T> void sbmv(long iSize, long iK,
                                T iAlpha, T* iA, T* iX, T iBeta, T* iY);

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
    CFLOAT dot<CFLOAT>(long iSize, CFLOAT* iX, CFLOAT* iY) {
        CFLOAT r;
        cblas_cdotc_sub(iSize, iX, 1, iY, 1, &r);
        return r;
    }
    template<>
    CDOUBLE dot<CDOUBLE>(long iSize, CDOUBLE* iX, CDOUBLE* iY) {
        CDOUBLE r;
        cblas_zdotc_sub(iSize, iX, 1, iY, 1, &r);
        return r;
    }

    template<>
    long iamax<float>(long iSize, float* iX) {
        return cblas_isamax(iSize, iX, 1);
    }
    template<>
    long iamax<double>(long iSize, double* iX) {
        return cblas_idamax(iSize, iX, 1);
    }
    template<>
    long iamax<CFLOAT>(long iSize, CFLOAT* iX) {
        return cblas_icamax(iSize, iX, 1);
    }
    template<>
    long iamax<CDOUBLE>(long iSize, CDOUBLE* iX) {
        return cblas_izamax(iSize, iX, 1);
    }

    template<>
    float asum<float>(long iSize, float* iX) {
        return cblas_sasum(iSize, iX, 1);
    }
    template<>
    double asum<double>(long iSize, double* iX) {
        return cblas_dasum(iSize, iX, 1);
    }

    template<>
    void tbmv<float>(long iSize, long iK, float* iA, float* iX) {
        cblas_stbmv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit,
                    iSize, iK, iA, 1, iX, 1);
    }
    template<>
    void tbmv<double>(long iSize, long iK, double* iA, double* iX) {
        cblas_dtbmv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit,
                    iSize, iK, iA, 1, iX, 1);
    }

    template<>
    void sbmv<float>(
        long iSize, long iK,
        float iAlpha, float* iA, float* iX, float iBeta, float* iY
    ) {
        cblas_ssbmv(CblasRowMajor, CblasUpper,
                    iSize, iK,
                    iAlpha, iA, 1, iX, 1, iBeta, iY, 1);
    }
    template<>
    void sbmv<double>(
        long iSize, long iK,
        double iAlpha, double* iA, double* iX, double iBeta, double* iY
    ) {
        cblas_dsbmv(CblasRowMajor, CblasUpper,
                    iSize, iK,
                    iAlpha, iA, 1, iX, 1, iBeta, iY, 1);
    }
}

#endif // CXXBLAS_H
