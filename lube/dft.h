/*
 * Copyright 2016 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#ifndef DFT_H
#define DFT_H

#include <lube/var.h>

namespace libube
{
    struct DFTImpl;

    /**
     * DFT functor implementation
     *
     * Uses the "pimpl" pattern: pointer to implementation.  This means the
     * implementation can be dependent upon whichever library is available at
     * compile time.
     *
     * Deals with both forward and inverse cases, which are normally just a
     * flag in the implementation library.  In general you should instantiate
     * either DFT or IDFT rather than this one.
     */
    class DFTBase : public UnaryFunctor
    {
    public:
        DFTBase(int iSize, bool iInverse, var iForwardType);
        ~DFTBase();
    protected:
        var alloc(var iVar) const;
        void scalar(const var& iVar, var& oVar) const;
        void vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const;
    private:
        DFTImpl* mImpl;
    };

    /**
     * DFT functor
     *
     * Instantiation of the DFT base class for the forward transform.
     */
    class DFT : public DFTBase
    {
    public:
        DFT(int iSize, var iForwardType=0.0f)
            : DFTBase(iSize, false, iForwardType) {};
    };

    /**
     * Inverse DFT functor
     *
     * Instantiation of the DFT base class for the backward transform.
     */
    class IDFT : public DFTBase
    {
    public:
        IDFT(int iSize, var iForwardType=0.0f)
            : DFTBase(iSize, true, iForwardType) {};
    };
}

#endif // DFT_H
