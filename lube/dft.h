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
     * DFT functor
     *
     * Uses the "pimpl" pattern: pointer to implementation.  This means the
     * implementation can be dependent upon whichever library is available at
     * compile time.
     */
    class DFT : public UnaryFunctor
    {
    public:
        DFT(int iSize, bool iInverse=false, var iForwardType=0.0f);
        ~DFT();
    protected:
        var alloc(var iVar) const;
        void scalar(const var& iVar, var& oVar) const;
        void vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const;
    private:
        DFTImpl* mImpl;
    };
}

#endif // DFT_H
