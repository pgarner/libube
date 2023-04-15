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


namespace libube
{
    /** Two vars */
    struct pair
    {
        var key;
        var val;
    };

    class Heap;

    /**
     * Heap interface
     */
    class IHeap
    {
    public:
        virtual ~IHeap() {};

        // Would be templates, but you can't do that in a virtual table
        virtual char* ptrchar(int iIndex=0) const = 0;
        virtual int* ptrint(int iIndex=0) const = 0;
        virtual long* ptrlong(int iIndex=0) const = 0;
        virtual float* ptrfloat(int iIndex=0) const = 0;
        virtual double* ptrdouble(int iIndex=0) const = 0;
        virtual cfloat* ptrcfloat(int iIndex=0) const = 0;
        virtual cdouble* ptrcdouble(int iIndex=0) const = 0;
        virtual var* ptrvar(int iIndex=0) const = 0;
        virtual pair* ptrpair(int iIndex=0) const = 0;

        // More template things
        virtual void append(int iSize, const char* iData) = 0;
        virtual void append(int iSize, const int* iData) = 0;
        virtual void append(int iSize, const cdouble* iData) = 0;

        // Trivial accessors
        virtual ind type() const = 0;
        virtual int size() const = 0;
        virtual int dim() const = 0;
        virtual int offset() const = 0;
        virtual int offset(int iOffset) = 0;
        virtual int& shape(int iDim) const = 0;
        virtual int& stride(int iDim) const = 0;

        // Methods
        virtual int attach() = 0;
        virtual int detach() = 0;
        virtual void resize(int iSize) = 0;
        virtual var at(int iIndex, bool iKey=false) const = 0;
        virtual var& key(int iIndex) = 0;
        virtual bool neq(IHeap* iHeap) = 0;
        virtual bool lt(IHeap* iHeap) = 0;
        virtual Heap* view() const = 0;
        virtual bool copyable(IHeap* iHeap) = 0;
        virtual var shift() = 0;
        virtual void unshift(var iVar) = 0;
        virtual bool defined(ind iIndex) = 0;
        virtual var* deref(ind iIndex) = 0;
        virtual int derefSize(ind iIndex) = 0;
        virtual ind derefType(ind iIndex) = 0;
        virtual ind derefAType(ind iIndex) = 0;
        virtual IHeap* derefHeap(ind iIndex) = 0;
        virtual var derefInt(ind iIndex, int iArrayIndex) = 0;
    };

    /**
     * Heap object managed by var
     *
     * It's just a reference counted array.  It would make sense to allocate
     * these from a pool, but for the moment they're done individually.
     */
    class Heap : public IHeap
    {
    public:
        // Special member functions
        Heap();
        virtual ~Heap();
        Heap(const IHeap& iHeap, bool iAllocOnly=false);

        // Should be templates
#define HPTRDECL(T, P) T* ptr##T(int iIndex=0) const {  \
            return mData.P + iIndex;                    \
        }
        HPTRDECL(char, cp)
        HPTRDECL(int, ip)
        HPTRDECL(long, lp)
        HPTRDECL(float, fp)
        HPTRDECL(double, dp)
        HPTRDECL(cfloat, cfp)
        HPTRDECL(cdouble, cdp)
        HPTRDECL(var, vp)
        HPTRDECL(pair, pp)

        // Overloaded constructors
        Heap(int iSize, ind iType);
        Heap(int iSize, const char* iData);
        Heap(int iSize, const int* iData);
        Heap(int iSize, const cdouble* iData);

        // Append: expand the array
        void append(int iSize, const char* iData);
        void append(int iSize, const int* iData);
        void append(int iSize, const cdouble* iData);

        // Trivial accessors
        virtual ind type() const { return mType; };
        virtual int size() const { return mSize; };
        virtual int dim() const { return 1; };
        virtual int offset() const { return 0; };
        virtual int offset(int iOffset) {
            throw error("Heap::offset(): not a view");
        };
        virtual int& shape(int iDim) const {
            throw error("Heap::shape(): not a view");
        };
        virtual int& stride(int iDim) const {
            throw error("Heap::stride(): not a view");
        };

        // Methods
        virtual int attach();
        virtual int detach();
        virtual void resize(int iSize);
        virtual var at(int iIndex, bool iKey=false) const;
        virtual var& key(int iIndex);
        virtual bool neq(IHeap* iHeap);
        virtual bool lt(IHeap* iHeap);
        virtual Heap* view() const { return 0; };
        virtual bool copyable(IHeap* iHeap) { return false; };
        virtual var shift();
        virtual void unshift(var iVar);
        virtual var* deref(ind iIndex);
        virtual bool defined(ind iIndex);
        virtual int derefSize(ind iIndex);
        virtual ind derefType(ind iIndex);
        virtual ind derefAType(ind iIndex);
        virtual IHeap* derefHeap(ind iIndex);
        virtual var derefInt(ind iIndex, int iArrayIndex);

    protected:
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

        dataType mData; ///< Pointer to allocated data
        int mSize;      ///< The externally visible size
        ind mType;      ///< The data type

        void copy(const Heap* iHeap, int iSize);

    private:
        // Members
        int mCapacity ; ///< The allocation size
        int mRefCount;  ///< Reference count

        // Methods
        template<class T> T* data() const;
        void alloc(int iSize);
        void dealloc(dataType iData);
    };


    /**
     * View object
     *
     * Essentially a Heap object, but the semantic is that it describes a view
     * of another Heap.  The data type is always int, the first element is the
     * offset, then the others occur in pairs, being the shape and stride
     * respectively.
     */
    class View : public Heap
    {
    public:
        View(IHeap* iHeap);
        View(IHeap* iHeap, const std::initializer_list<int> iList, int iOffset);
        View(IHeap* iHeap, var iShape, int iOffset);
        View(const IHeap& iHeap, bool iAllocOnly=false);
        virtual ~View();

        // Should be templates
#define VPTRDECL(T) T* ptr##T(int iIndex=0) const {             \
            return mHeap->ptr##T(iIndex + mData.ip[0]);         \
        }
        VPTRDECL(char)
        VPTRDECL(int)
        VPTRDECL(long)
        VPTRDECL(float)
        VPTRDECL(double)
        VPTRDECL(cfloat)
        VPTRDECL(cdouble)
        VPTRDECL(var)
        VPTRDECL(pair)

        // Trivial accessors
        virtual int size() const { return stride(0) * shape(0); };
        virtual ind type() const { return mHeap->type(); };
        virtual int dim() const { return (mSize-1) / 2; };
        virtual int offset() const { return mData.ip[0]; };
        virtual int offset(int iOffset);
        virtual Heap* view() const { return mHeap; };
        virtual int& shape(int iDim) const;
        virtual int& stride(int iDim) const;
        virtual bool copyable(IHeap* iHeap);
        virtual int derefSize(ind iIndex) {
            return mHeap->derefSize(iIndex + mData.ip[0]);
        };
        virtual ind derefType(ind iIndex) {
            return mHeap->derefType(iIndex + mData.ip[0]);
        };
        virtual ind derefAType(ind iIndex) {
            return mHeap->derefAType(iIndex + mData.ip[0]);
        };

    private:
        void setStrides(int iDim);
        Heap* mHeap;    ///< The real storage
    };
}

#endif // HEAP_H
