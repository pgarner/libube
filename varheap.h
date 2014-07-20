/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#ifndef VARHEAP_H
#define VARHEAP_H

#include "var.h"


namespace libvar
{
    /** Two vars */
    struct pair
    {
        var key;
        var val;
    };


    /**
     * Heap object managed by var
     *
     * It's just a reference counted array.  It would make sense to allocate
     * these from a pool, but for the moment they're done individually.
     */
    class varheap
    {
    public:

        // Special member functions
        varheap();
        ~varheap();
        varheap(const varheap& iHeap, bool iAllocOnly=false);

        // Templates
        template<class T> T* ptr(int iIndex=0) const;

        // Overloaded constructors
        varheap(int iSize, ind iType);
        varheap(int iSize, const char* iData);
        varheap(int iSize, const int* iData);
        varheap(int iSize, const cdouble* iData);

        // Trivial accessors
        ind type() const { return mView ? mView->type() : mType; };
        int size() const;
        int dim() const { return mView ? (mSize-1) / 2 : 1; };
        int offset() const { return mView ? mData.ip[0] : 0; };
        int offset(int iOffset);
        int& shape(int iDim) const;
        int& stride(int iDim) const;

        // Methods
        int attach();
        int detach();
        void resize(int iSize);
        long double strtold();
        void format(std::ostream& iStream, int iIndent = 0);
        var at(int iIndex, bool iKey=false) const;
        var& key(int iIndex);
        bool neq(varheap* iHeap);
        bool lt(varheap* iHeap);
        void setView(varheap* iVarHeap);
        bool view() const { return mView; };
        bool copyable(varheap* iHeap);

    private:
    
        union dataType {
            char* cp;
            int* ip;
            long* lp;
            float* fp;
            double* dp;
            cfloat* cfp;
            cdouble* cdp;
            var* vp;
            pair* pp;
        };

        // Members
        dataType mData; ///< Pointer to allocated data
        int mSize;      ///< The externally visible size
        int mCapacity ; ///< The allocation size
        int mRefCount;  ///< Reference count
        ind mType;      ///< The data type
        varheap* mView; ///< If this is a view, the real storage

        // Methods
        void copy(const varheap* iHeap, int iSize);
        void alloc(int iSize);
        void dealloc(dataType iData);
        void formatView(std::ostream& iStream);
    };
}

#endif // VARHEAP_H
