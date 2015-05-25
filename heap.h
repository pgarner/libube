/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#ifndef HEAP_H
#define HEAP_H

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
    class Heap
    {
    public:

        // Special member functions
        Heap();
        ~Heap();
        Heap(const Heap& iHeap, bool iAllocOnly=false);

        // Templates
        template<class T> T* ptr(int iIndex=0) const {
            return mView
                ? mView->ptr<T>(iIndex + mData.ip[0])
                : data<T>() + iIndex;
        }

        // Overloaded constructors
        Heap(int iSize, ind iType);
        Heap(int iSize, const char* iData);
        Heap(int iSize, const int* iData);
        Heap(int iSize, const cdouble* iData);

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
        void format(std::ostream& iStream, int iIndent = 0);
        var at(int iIndex, bool iKey=false) const;
        var& key(int iIndex);
        bool neq(Heap* iHeap);
        bool lt(Heap* iHeap);
        void setView(Heap* iHeap);
        bool view() const { return mView; };
        bool copyable(Heap* iHeap);
        var shift();
        void unshift(var iVar);

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
        Heap* mView; ///< If this is a view, the real storage

        // Methods
        template<class T> T* data() const;
        void copy(const Heap* iHeap, int iSize);
        void alloc(int iSize);
        void dealloc(dataType iData);
        void formatView(std::ostream& iStream, int iIndent);
    };
}

#endif // HEAP_H
