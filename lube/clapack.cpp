/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2015
 */

#include <complex>

#include "clapack.h"
#include "c++blas.h"
#include "c++lapack.h"

static char sV = 'V';
static char sN = 'N';
static char sT = 'T';
static char sL = 'L';
static integer sOne = 1L;

#define CFLOAT std::complex<float>
#define CDOUBLE std::complex<double>

namespace blas
{
    template<>
    void swap<float>(long iN, float* iX, float* oY)
    {
        sswap_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void swap<double>(long iN, double* iX, double* oY)
    {
        dswap_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void swap<CFLOAT>(long iN, CFLOAT* iX, CFLOAT* oY)
    {
        cswap_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void swap<CDOUBLE>(long iN, CDOUBLE* iX, CDOUBLE* oY)
    {
        zswap_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void copy<float>(long iN, float* iX, float* oY)
    {
        scopy_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void copy<double>(long iN, double* iX, double* oY)
    {
        dcopy_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void copy<CFLOAT>(long iN, CFLOAT* iX, CFLOAT* oY)
    {
        ccopy_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void copy<CDOUBLE>(long iN, CDOUBLE* iX, CDOUBLE* oY)
    {
        zcopy_(&iN, iX, &sOne, oY, &sOne);
    }

    template<>
    void axpy<float>(long iN, float iAlpha, float* iX, float* ioY)
    {
        saxpy_(&iN, &iAlpha, iX, &sOne, ioY, &sOne);
    }

    template<>
    void axpy<double>(long iN, double iAlpha, double* iX, double* ioY)
    {
        daxpy_(&iN, &iAlpha, iX, &sOne, ioY, &sOne);
    }

    template<>
    void axpy<CFLOAT>(long iN, CFLOAT iAlpha, CFLOAT* iX, CFLOAT* ioY)
    {
        caxpy_(&iN, &iAlpha, iX, &sOne, ioY, &sOne);
    }

    template<>
    void axpy<CDOUBLE>(long iN, CDOUBLE iAlpha, CDOUBLE* iX, CDOUBLE* ioY)
    {
        zaxpy_(&iN, &iAlpha, iX, &sOne, ioY, &sOne);
    }


    template<>
    void scal<float>(long iN, float iAlpha, float* ioX)
    {
        sscal_(&iN, &iAlpha, ioX, &sOne);
    }

    template<>
    void scal<double>(long iN, double iAlpha, double* ioX)
    {
        dscal_(&iN, &iAlpha, ioX, &sOne);
    }

    template<>
    float dot<float>(long iN, float* iX, float* iY)
    {
        return sdot_(&iN, iX, &sOne, iY, &sOne);
    }

    template<>
    double dot<double>(long iN, double* iX, double* iY)
    {
        return ddot_(&iN, iX, &sOne, iY, &sOne);
    }

    template<>
    CFLOAT dot<CFLOAT>(long iN, CFLOAT* iX, CFLOAT* iY)
    {
        CFLOAT r;
        cdotc_(&r, &iN, iX, &sOne, iY, &sOne);
        return r;
    }

    template<>
    CDOUBLE dot<CDOUBLE>(long iN, CDOUBLE* iX, CDOUBLE* iY)
    {
        CDOUBLE r;
        zdotc_(&r, &iN, iX, &sOne, iY, &sOne);
        return r;
    }

    template<>
    long iamax<float>(long iN, float* iX)
    {
        return isamax_(&iN, iX, &sOne) - 1;
    }

    template<>
    long iamax<double>(long iN, double* iX)
    {
        return idamax_(&iN, iX, &sOne) - 1;
    }

    template<>
    long iamax<CFLOAT>(long iN, CFLOAT* iX)
    {
        return icamax_(&iN, iX, &sOne) - 1;
    }

    template<>
    long iamax<CDOUBLE>(long iN, CDOUBLE* iX)
    {
        return izamax_(&iN, iX, &sOne) - 1;
    }

    template<>
    float asum<float>(long iN, float* iX)
    {
        return sasum_(&iN, iX, &sOne);
    }

    template<>
    double asum<double>(long iN, double* iX)
    {
        return dasum_(&iN, iX, &sOne);
    }

    template<>
    void tbmv<float>(long iN, long iK, float* iA, float* ioX)
    {
        stbmv_(&sL, &sT, &sN, &iN, &iK, iA, &sOne, ioX, &sOne);
    }

    template<>
    void tbmv<double>(long iN, long iK, double* iA, double* ioX)
    {
        dtbmv_(&sL, &sT, &sN, &iN, &iK, iA, &sOne, ioX, &sOne);
    }

    template<>
    void sbmv<float>(
        long iN, long iK,
        float iAlpha, float* iA, float* iX, float iBeta, float* ioY
    )
    {
        ssbmv_(&sL, &iN, &iK,
               &iAlpha, iA, &sOne, iX, &sOne, &iBeta, ioY, &sOne);
    }

    template<>
    void sbmv<double>(
        long iN, long iK,
        double iAlpha, double* iA, double* iX, double iBeta, double* ioY
    )
    {
        dsbmv_(&sL, &iN, &iK,
               &iAlpha, iA, &sOne, iX, &sOne, &iBeta, ioY, &sOne);
    }

    template<>
    void gemm<float>(
        long iM, long iN, long iK,
        float iAlpha, float* iA, float* iB,
        float iBeta, float* ioC
    )
    {
        sgemm_(&sN, &sN,
               &iN, &iM, &iK,
               &iAlpha, iB, &iN, iA, &iK,
               &iBeta, ioC, &iN);
    }
    template<>
    void gemm<double>(
        long iM, long iN, long iK,
        double iAlpha, double* iA, double* iB,
        double iBeta, double* ioC
    )
    {
        dgemm_(&sN, &sN,
               &iN, &iM, &iK,
               &iAlpha, iB, &iN, iA, &iK,
               &iBeta, ioC, &iN);
    }
}


namespace lapack
{
    template<>
    long gees<float>(
        long n, float* a, float* wr, float* wi, float* vs
    )
    {
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

    template<>
    long geev<float>(
        long n, float* a, float* wr, float* wi, float* vl, float* vr
    )
    {
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

    template<>
    long geev<double>(
        long n, double* a, double* wr, double* wi, double* vl, double* vr
    )
    {
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
