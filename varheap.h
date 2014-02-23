/*
 * Copyright 2013 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#ifndef VARHEAP_H
#define VARHEAP_H

#include "var.h"


/** Two vars */
struct pair
{
    var key;
    var val;
};


/**
 * Heap object managed by var
 *
 * It's just a reference counted array.  It would make sense to
 * allocate these from a pool, but for the moment they're done
 * individually.
 */
class varheap
{
public:

    // Special member functions
    varheap();
    ~varheap();
    varheap(const varheap& iHeap);

    // Templates
    template<class T> T& ref(int iIndex) const;

    // Overloaded constructors
    varheap(int iSize, var::dataEnum iType);
    varheap(int iSize, const char* iData);
    varheap(int iSize, const int* iData);

    // Trivial accessors
    var::dataEnum type() const { return mView ? mView->type() : mType; };
    int size() const;
    int dim() const { return mView ? (mSize-1) / 2 : 1; };
    int offset() const { return mView ? mData.ip[0] : 0; };
    int offset(int iOffset);
    int shape(int iDim) const;
    int stride(int iDim) const;
    char* ref() const { return mData.cp; };

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
    bool view() { return mView; };
    int& viewRef(int iIndex);
    bool copyable(varheap* iHeap);

    // Maths
    void pow(var iPower);

    // Vector ops
    void set(const varheap* iHeap, int iOffset, int iSize);
    void add(const varheap* iHeap, int iOffset, int iSize);
    void sub(const varheap* iHeap, int iOffset, int iSize);
    void scal(int iSize, int iOffset, var iVar);
    void mul(
        int iM, int iN, int iK, int iOffset,
        const varheap* iHeapA, int iOffsetA, varheap* iHeapB
    );
    var asum();

private:
    
    union dataType {
        char* cp;
        int* ip;
        long* lp;
        float* fp;
        double* dp;
        var* vp;
        pair* pp;
    };

    // Members
    dataType mData; ///< Pointer to allocated data
    int mSize;      ///< The externally visible size
    int mCapacity ; ///< The allocation size
    int mRefCount;  ///< Reference count
    var::dataEnum mType; ///< The data type
    varheap* mView; ///< If this is a view, the real storage

    // Methods
    void copy(const varheap* iHeap, int iSize);
    void alloc(int iSize);
    void dealloc(dataType iData);
    void formatView(std::ostream& iStream);
};

#endif // VARHEAP_H
