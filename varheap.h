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

    // Overloaded constructors
    varheap(int iSize, var::dataEnum iType);
    varheap(int iSize, const char* iData);
    varheap(int iSize, const int* iData);

    // Chums
    friend class var;
    friend std::ostream& operator <<(
        std::ostream& iStream, const var& iVar
    );

    // Methods
    int size();
    int attach();
    int detach();
    long double strtold();
    var at(int iIndex) const;

private:
    
    union dataType {
        var* vp;
        char* cp;
        int* ip;
        long* lp;
        float* fp;
        double* dp;
    };

    // Members
    dataType mData; ///< Pointer to allocated data
    int mSize;      ///< The externally visible size
    int mCapacity ; ///< The allocation size
    int mRefCount;  ///< Reference count
    var::dataEnum mType; ///< The data type

    // Methods
    void resize(int iSize);
    void alloc(int iSize);
    void dealloc(dataType iData);
};

#endif // VARHEAP_H
