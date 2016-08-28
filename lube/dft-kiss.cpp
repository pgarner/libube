/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

#include <cassert>

#include "kiss_fft.h"
#include "kiss_fftr.h"

#include "lube/dft.h"


namespace kissfft
{
    static int sInstanceCount = 0;
};


/**
 * The Kiss DFT implementation
 */
struct libube::DFTImpl
{
    void* config;
    var forwardType;
    var inverseType;
    int oSize;
    bool inverse;
};


using namespace libube;


/**
 * DFTBase constructor
 *
 * The default forward type is 0.0f, meaning that it defaults to a single
 * precision real transform.  The output type is always complex; in the case of
 * a real transform the size of the complex output is iSize/2+1.
 */
DFTBase::DFTBase(int iSize, bool iInverse, var iForwardType)
{
    mImpl = new DFTImpl;

    // It's a 1 dimensional thing (for now)
    mDim = 1;
    mImpl->config = 0;
    mImpl->inverse = iInverse;
    mImpl->forwardType = iForwardType;

    // Set the input and output types for a forward transform
    switch (mImpl->forwardType.type())
    {
    case TYPE_FLOAT:
        mImpl->inverseType = cfloat(0.0f, 0.0f);
        mImpl->oSize = iInverse ? iSize : iSize / 2 + 1;
        if (iInverse)
            mImpl->config = kiss_fftr_alloc(iSize, 1, 0, 0);
        else
            mImpl->config = kiss_fftr_alloc(iSize, 0, 0, 0);
        break;
    case TYPE_DOUBLE:
        throw error("DFTBase::DFTBase: Kiss FFT float only");
        break;
    case TYPE_CFLOAT:
        mImpl->inverseType = cfloat(0.0f, 0.0f);
        mImpl->oSize = iSize;
        if (iInverse)
            mImpl->config = kiss_fft_alloc(iSize, 1, 0, 0);
        else
            mImpl->config = kiss_fft_alloc(iSize, 0, 0, 0);
        break;
    case TYPE_CDOUBLE:
        throw error("DFTBase::DFTBase: Kiss FFT float only");
        break;
    default:
        throw error("DFTBase::DFTBase: Unknown type");
    }
    
    // Update the instance count
    kissfft::sInstanceCount++;
}

DFTBase::~DFTBase()
{
    free(mImpl->config);
    delete mImpl;
    mImpl = 0;

    if (--kissfft::sInstanceCount == 0)
        kiss_fft_cleanup();
}

var DFTBase::alloc(var iVar) const
{
    // Allocate an output array
    var r;
    var s = iVar.shape();
    s[s.size()-1] = mImpl->oSize;
    r = view(s, mImpl->inverse ? mImpl->forwardType : mImpl->inverseType);
    return r;
}

void DFTBase::scalar(const var& iVar, var& oVar) const
{
    // Check the arrays are OK
    if (iVar.type() != TYPE_ARRAY)
        throw error("DFTBase::scalar: DFTBase input must be vector");
    if (oVar.type() != TYPE_ARRAY)
        throw error("DFTBase::scalar: DFTBase output must be vector");
    if (iVar.atype() != (mImpl->inverse
                         ? mImpl->inverseType.type()
                         : mImpl->forwardType.type()
        ))
        throw error("DFTBase::scalar: wrong input type");
    if (oVar.atype() != (mImpl->inverse
                          ? mImpl->forwardType.type()
                          : mImpl->inverseType.type()
        ))
        throw error("DFTBase::scalar: wrong output type");

    // DFTBase always broadcasts to vector()
    broadcast(iVar, oVar);

    // Kiss does not include the divide by N on the inverse
    if (mImpl->inverse)
        oVar /= mImpl->oSize;
}

void DFTBase::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(oVar);
    if (iVar.is(oVar))
    {
        throw error("DFTBase::vector(): not implemented");
    }
    else
        if (mImpl->inverse)
            switch (mImpl->forwardType.type())
            {
            case TYPE_FLOAT:
                kiss_fftri(
                    (kiss_fftr_cfg)mImpl->config,
                    (const kiss_fft_cpx*)iVar.ptr<cfloat>(iOffsetI),
                    (float*)oVar.ptr<float>(iOffsetO)
                );
                break;
            case TYPE_DOUBLE:
                throw error("DFTBase::vector(): Kiss FFT float only");
                break;
            case TYPE_CFLOAT:
                kiss_fft(
                    (kiss_fft_cfg)mImpl->config,
                    (const kiss_fft_cpx*)iVar.ptr<cfloat>(iOffsetI),
                    (kiss_fft_cpx*)oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_CDOUBLE:
                throw error("DFTBase::vector(): Kiss FFT float only");
                break;
            default:
                throw error("DFTBase::vector(): unknown type");
            }
        else
            switch (mImpl->forwardType.type())
            {
            case TYPE_FLOAT:
                kiss_fftr(
                    (kiss_fftr_cfg)mImpl->config,
                    (const float*)iVar.ptr<float>(iOffsetI),
                    (kiss_fft_cpx*)oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_DOUBLE:
                throw error("DFTBase::vector(): Kiss FFT float only");
                break;
            case TYPE_CFLOAT:
                kiss_fft(
                    (kiss_fft_cfg)mImpl->config,
                    (const kiss_fft_cpx*)iVar.ptr<cfloat>(iOffsetI),
                    (kiss_fft_cpx*)oVar.ptr<cfloat>(iOffsetO)
                );
                break;
            case TYPE_CDOUBLE:
                throw error("DFTBase::vector(): Kiss FFT float only");
                break;
            default:
                throw error("DFTBase::vector(): unknown type");
            }
}
