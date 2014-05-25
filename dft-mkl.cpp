/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

#include <cassert>
#include <mkl.h>
#include "var.h"
#include "varheap.h" // for heap()->type()

void dftiCheck(MKL_LONG iReturn)
{
    if (iReturn == DFTI_NO_ERROR)
        return;
    throw std::runtime_error(DftiErrorMessage(iReturn));
}

DFT::DFT(int iSize, bool iInverse, bool iComplex, bool iDouble)
{
    // It's a 1 dimensional thing (for now)
    mDim = 1;

    // Set the input and output types for a forward transform
    if (iComplex)
    {
        mOSize = iSize;
        if (iDouble)
        {
            mIType = var::TYPE_CDOUBLE;
            mOType = var::TYPE_CDOUBLE;
        }
        else
        {
            mIType = var::TYPE_CFLOAT;
            mOType = var::TYPE_CFLOAT;
        }
    }
    else
    {
        mOSize = iInverse ? iSize : iSize / 2 + 1;
        if (iDouble)
        {
            mIType = var::TYPE_DOUBLE;
            mOType = var::TYPE_CDOUBLE;
        }
        else
        {
            mIType = var::TYPE_FLOAT;
            mOType = var::TYPE_CFLOAT;
        }
    }

    // Swap the types if it's an inverse transform
    mInverse = iInverse;
    if (iInverse)
    {
        var::dataEnum tmp;
        tmp = mIType;
        mIType = mOType;
        mOType = tmp;
    }

    // Create and commit the DFTI descriptor
    MKL_LONG r;
    mHandle = 0;
    r = DftiCreateDescriptor(
        &mHandle,
        iDouble ? DFTI_DOUBLE : DFTI_SINGLE,
        iComplex ? DFTI_COMPLEX : DFTI_REAL,
        1, iSize
    );
    dftiCheck(r);

    // Default is to overwrite the input
    r = DftiSetValue(mHandle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
    dftiCheck(r);

    r = DftiCommitDescriptor(mHandle);
    dftiCheck(r);
}

DFT::~DFT()
{
    MKL_LONG r = DftiFreeDescriptor(&mHandle);
    dftiCheck(r);
}

var DFT::operator ()(const var& iVar, var* oVar) const
{
    var r;
    if (!oVar)
    {
        // Allocate an output array
        var t;
        switch (mOType)
        {
        case var::TYPE_FLOAT:
            t = 0.0f;
            break;
        case var::TYPE_DOUBLE:
            t = 0.0;
            break;
        case var::TYPE_CFLOAT:
            t = std::complex<float>(0.0f, 0.0f);
            break;
        case var::TYPE_CDOUBLE:
            t = std::complex<double>(0.0, 0.0);
            break;
        default:
            assert(0);
        }
        var s = iVar.shape();
        s[s.size()-1] = mOSize;
        r = view(s, t);
        oVar = &r;
    }

    // Check the arrays are OK
    if (iVar.type() != var::TYPE_ARRAY)
        throw std::runtime_error("DFT::operator(): DFT input must be vector");
    if (oVar->type() != var::TYPE_ARRAY)
        throw std::runtime_error("DFT::operator(): DFT output must be vector");
    if (iVar.heap()->type() != mIType)
        throw std::runtime_error("DFT::operator(): wrong input type");
    if (oVar->heap()->type() != mOType)
        throw std::runtime_error("DFT::operator(): wrong output type");

    // DFT always broadcasts to array()
    broadcast(iVar, oVar);
    return *oVar;
}

void DFT::array(var iVar, var* oVar, int iIndex) const
{
    assert(oVar);
    MKL_LONG r;
    int iVarOffset = iVar.stride(iVar.dim()-mDim) * iIndex;
    int oVarOffset = oVar->stride(oVar->dim()-mDim) * iIndex;
    if (iVar.is(*oVar))
        if (mInverse)
            r = DftiComputeBackward(mHandle, oVar->ptr<float>()+oVarOffset);
        else
            r = DftiComputeForward(mHandle, oVar->ptr<float>()+oVarOffset);
    else
        if (mInverse)
            r = DftiComputeBackward(
                mHandle,
                iVar.ptr<float>()+iVarOffset, oVar->ptr<float>()+oVarOffset
            );
        else
            r = DftiComputeForward(
                mHandle,
                iVar.ptr<float>()+iVarOffset, oVar->ptr<float>()+oVarOffset
            );
    dftiCheck(r);
}
