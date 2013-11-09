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

#include <var>


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

    // Overloaded constructors
    varheap(int iSize, var::dataEnum iType);
    varheap(int iSize, const char* iData);
    varheap(int iSize, const int* iData);

    // Chums
    friend class var;

    // Methods
    int size();
    int attach();
    int detach();
    void resize(int iSize);
    long double strtold();
    void format(std::ostream& iStream);
    void formatview(std::ostream& iStream);
    var at(int iIndex, bool iKey=false) const;
    void set(var iVar, int iIndex=-1, bool iKey=false);
    void add(var iVar, int iIndex=-1);
    void sub(var iVar, int iIndex=-1);
    void mul(var iVar, int iIndex=-1);
    void div(var iVar, int iIndex=-1);
    void setView(varheap* iVarHeap);

    // Maths
    void pow(var iPower);

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
    void alloc(int iSize);
    void dealloc(dataType iData);
};

#endif // VARHEAP_H
