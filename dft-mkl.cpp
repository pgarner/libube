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


using namespace libvar;


void dftiCheck(MKL_LONG iReturn)
{
    if (iReturn == DFTI_NO_ERROR)
        return;
    throw std::runtime_error(DftiErrorMessage(iReturn));
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
    // It's a 1 dimensional thing (for now)
    mDim = 1;
    mHandle = 0;
    mInverse = iInverse;
    mForwardType = iForwardType; 

    // Set the input and output types for a forward transform
    MKL_LONG r;
    switch (mForwardType.type())
    {
    case TYPE_FLOAT:
        mInverseType = cfloat(0.0f, 0.0f);
        mOSize = iInverse ? iSize : iSize / 2 + 1;
        r = DftiCreateDescriptor(&mHandle, DFTI_SINGLE, DFTI_REAL, 1, iSize);
        break;
    case TYPE_DOUBLE:
        mInverseType = cdouble(0.0, 0.0);
        mOSize = iInverse ? iSize : iSize / 2 + 1;
        r = DftiCreateDescriptor(&mHandle, DFTI_DOUBLE, DFTI_REAL, 1, iSize);
        break;
    case TYPE_CFLOAT:
        mInverseType = cfloat(0.0f, 0.0f);
        mOSize = iSize;
        r = DftiCreateDescriptor(&mHandle, DFTI_SINGLE, DFTI_COMPLEX, 1, iSize);
        break;
    case TYPE_CDOUBLE:
        mInverseType = cdouble(0.0, 0.0);
        mOSize = iSize;
        r = DftiCreateDescriptor(&mHandle, DFTI_DOUBLE, DFTI_COMPLEX, 1, iSize);
        break;
    default:
        throw std::runtime_error("DFT::DFT: Unknown type");
    }
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
        var s = iVar.shape();
        s[s.size()-1] = mOSize;
        r = view(s, mInverse ? mForwardType : mInverseType);
        oVar = &r;
    }

    // Check the arrays are OK
    if (iVar.type() != TYPE_ARRAY)
        throw std::runtime_error("DFT::operator(): DFT input must be vector");
    if (oVar->type() != TYPE_ARRAY)
        throw std::runtime_error("DFT::operator(): DFT output must be vector");
    if (iVar.heap()->type() !=
        (mInverse ? mInverseType.type() : mForwardType.type()))
        throw std::runtime_error("DFT::operator(): wrong input type");
    if (oVar->heap()->type() !=
        (mInverse ? mForwardType.type() : mInverseType.type()))
        throw std::runtime_error("DFT::operator(): wrong output type");

    // DFT always broadcasts to array()
    broadcast(iVar, oVar);
    return *oVar;
}

void DFT::array(var iVar, var* oVar, int iIndex) const
{
    assert(oVar);
    MKL_LONG r;
    int iVarOffset = iVar.stride(iVar.dim()-mDim-1) * iIndex;
    int oVarOffset = oVar->stride(oVar->dim()-mDim-1) * iIndex;
    if (iVar.is(*oVar))
    {
        throw std::runtime_error("DFT::array(): not implemented");
        if (mInverse)
            r = DftiComputeBackward(mHandle, oVar->ptr<float>()+oVarOffset);
        else
            r = DftiComputeForward(mHandle, oVar->ptr<float>()+oVarOffset);
    }
    else
        if (mInverse)
            r = DftiComputeBackward(
                mHandle,
                iVar.ptr<float>()+iVarOffset, oVar->ptr<float>()+oVarOffset
            );
        else
            switch (mForwardType.type())
            {
            case TYPE_FLOAT:
                r = DftiComputeForward(
                    mHandle,
                    iVar.ptr<float>() + iVarOffset,
                    oVar->ptr<cfloat>() + oVarOffset
                );
                break;
            case TYPE_DOUBLE:
                r = DftiComputeForward(
                    mHandle,
                    iVar.ptr<double>() + iVarOffset,
                    oVar->ptr<cdouble>() + oVarOffset
                );
                break;
            case TYPE_CFLOAT:
                r = DftiComputeForward(
                    mHandle,
                    iVar.ptr<cfloat>() + iVarOffset,
                    oVar->ptr<cfloat>() + oVarOffset
                );
                break;
            case TYPE_CDOUBLE:
                r = DftiComputeForward(
                    mHandle,
                    iVar.ptr<cdouble>() + iVarOffset,
                    oVar->ptr<cdouble>() + oVarOffset
                );
                break;
            default:
                throw std::runtime_error("DFT::array(): unknown type");
            }
    dftiCheck(r);
}
