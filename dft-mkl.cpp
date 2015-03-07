/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

#include <cassert>
#include <mkl_dfti.h>

#include "var.h"


/**
 * The MKL DFT implementation (aka DFTI)
 */
struct libvar::DFTImpl
{
    DFTI_DESCRIPTOR_HANDLE handle;
    var forwardType;
    var inverseType;
    int oSize;
    bool inverse;
};


using namespace libvar;


void dftiCheck(MKL_LONG iReturn)
{
    if (iReturn == DFTI_NO_ERROR)
        return;
    throw error(DftiErrorMessage(iReturn));
}


/**
 * DFT constructor
 *
 * The default forward type is 0.0f, meaning that it defaults to a single
 * precision real transform.  The output type is always complex; in the case of
 * a real transform the size of the complex output is iSize/2+1.
 */
DFT::DFT(int iSize, bool iInverse, var iForwardType)
{
    mImpl = new DFTImpl;

    // It's a 1 dimensional thing (for now)
    mDim = 1;
    mImpl->handle = 0;
    mImpl->inverse = iInverse;
    mImpl->forwardType = iForwardType;

    // Set the input and output types for a forward transform
    MKL_LONG r;
    switch (mImpl->forwardType.type())
    {
    case TYPE_FLOAT:
        mImpl->inverseType = cfloat(0.0f, 0.0f);
        mImpl->oSize = iInverse ? iSize : iSize / 2 + 1;
        r = DftiCreateDescriptor(
            &mImpl->handle, DFTI_SINGLE, DFTI_REAL, 1, iSize
        );
        break;
    case TYPE_DOUBLE:
        mImpl->inverseType = cdouble(0.0, 0.0);
        mImpl->oSize = iInverse ? iSize : iSize / 2 + 1;
        r = DftiCreateDescriptor(
            &mImpl->handle, DFTI_DOUBLE, DFTI_REAL, 1, iSize
        );
        break;
    case TYPE_CFLOAT:
        mImpl->inverseType = cfloat(0.0f, 0.0f);
        mImpl->oSize = iSize;
        r = DftiCreateDescriptor(
            &mImpl->handle, DFTI_SINGLE, DFTI_COMPLEX, 1, iSize
        );
        break;
    case TYPE_CDOUBLE:
        mImpl->inverseType = cdouble(0.0, 0.0);
        mImpl->oSize = iSize;
        r = DftiCreateDescriptor(
            &mImpl->handle, DFTI_DOUBLE, DFTI_COMPLEX, 1, iSize
        );
        break;
    default:
        throw error("DFT::DFT: Unknown type");
    }
    dftiCheck(r);

    // Default is to overwrite the input
    r = DftiSetValue(mImpl->handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
    dftiCheck(r);

    r = DftiCommitDescriptor(mImpl->handle);
    dftiCheck(r);
}

DFT::~DFT()
{
    MKL_LONG r = DftiFreeDescriptor(&mImpl->handle);
    dftiCheck(r);
    delete mImpl;
    mImpl = 0;
}

var DFT::alloc(var iVar) const
{
    // Allocate an output array
    var r;
    var s = iVar.shape();
    s[s.size()-1] = mImpl->oSize;
    r = view(s, mImpl->inverse ? mImpl->forwardType : mImpl->inverseType);
    return r;
}

void DFT::scalar(const var& iVar, var& oVar) const
{
    // Check the arrays are OK
    if (iVar.type() != TYPE_ARRAY)
        throw error("DFT::scalar: DFT input must be vector");
    if (oVar.type() != TYPE_ARRAY)
        throw error("DFT::scalar: DFT output must be vector");
    if (iVar.atype() != (mImpl->inverse
                         ? mImpl->inverseType.type()
                         : mImpl->forwardType.type()
        ))
        throw error("DFT::scalar: wrong input type");
    if (oVar.atype() != (mImpl->inverse
                          ? mImpl->forwardType.type()
                          : mImpl->inverseType.type()
        ))
        throw error("DFT::scalar: wrong output type");

    // DFT always broadcasts to vector()
    broadcast(iVar, oVar);
}

void DFT::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(oVar);
    MKL_LONG r;
    if (iVar.is(oVar))
    {
        throw error("DFT::vector(): not implemented");
        if (mImpl->inverse)
            r = DftiComputeBackward(mImpl->handle, oVar.ptr<float>(iOffsetO));
        else
            r = DftiComputeForward(mImpl->handle, oVar.ptr<float>(iOffsetO));
    }
    else
        if (mImpl->inverse)
            switch (mImpl->forwardType.type())
            {
            case TYPE_FLOAT:
                r = DftiComputeBackward(
                    mImpl->handle,
                    iVar.ptr<cfloat>(iOffsetI),
                    oVar.ptr<float>(iOffsetO)
                );
                break;
            case TYPE_DOUBLE:
                r = DftiComputeBackward(
                    mImpl->handle,
                    iVar.ptr<cdouble>(iOffsetI),
                    oVar.ptr<double>(iOffsetO)
                );
                break;
            case TYPE_CFLOAT:
                r = DftiComputeBackward(
                    mImpl->handle,
                    iVar.ptr<cfloat>(iOffsetI),
                    oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_CDOUBLE:
                r = DftiComputeBackward(
                    mImpl->handle,
                    iVar.ptr<cdouble>(iOffsetI),
                    oVar.ptr<cdouble>(iOffsetO)
                );
                break;
            default:
                throw error("DFT::vector(): unknown type");
            }
        else
            switch (mImpl->forwardType.type())
            {
            case TYPE_FLOAT:
                r = DftiComputeForward(
                    mImpl->handle,
                    iVar.ptr<float>(iOffsetI),
                    oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_DOUBLE:
                r = DftiComputeForward(
                    mImpl->handle,
                    iVar.ptr<double>(iOffsetI),
                    oVar.ptr<cdouble>(iOffsetO)
                );
                break;
            case TYPE_CFLOAT:
                r = DftiComputeForward(
                    mImpl->handle,
                    iVar.ptr<cfloat>(iOffsetI),
                    oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_CDOUBLE:
                r = DftiComputeForward(
                    mImpl->handle,
                    iVar.ptr<cdouble>(iOffsetI),
                    oVar.ptr<cdouble>(iOffsetO)
                );
                break;
            default:
                throw error("DFT::vector(): unknown type");
            }
    dftiCheck(r);
}
