/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2013
 */

#ifndef VAR_H
#define VAR_H

#include <iostream>

class varheap;

/**
 * Class with runtime type determination.
 *
 * The name 'var' is borrowed from ECMAScript.  The syntax is intended
 * to be more from ruby, but we can't use delete() method!() or
 * method?().
 */
class var
{
public:

    /** The possible var types */
    enum dataEnum {
        TYPE_ARRAY = 0,
        TYPE_CHAR,
        TYPE_INT,
        TYPE_LONG,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_VAR
    };

    // Special member functions
    var();
    ~var();
    var(const var& iVar);
    var& operator =(const var& iVar);

    // Overloaded constructors
    var(char iData);
    var(int iData);
    var(long iData);
    var(float iData);
    var(double iData);
    var(const char* iData);
    var(int iSize, const char* iData);
    var(int iSize, char* const* iData);
    var(int iSize, const int* iData);

    // Operators
    bool operator ==(const var& iVar) const;
    bool operator !=(const var& iVar) const;
    bool operator <(const var& iVar) const;
    var& operator +=(const var& iVar);
    var& operator -=(const var& iVar);
    const var operator [](int iIndex) const { return at(iIndex); };
    var operator [](int iIndex);
    char* operator &();

    // Methods
    var copy() const;
    bool defined() const;
    int size() const;
    dataEnum type() const;
    template<class T> T cast();
    bool heap(int iSize = -1) const;
    void set(int iIndex, var iVar);
    var at(int iIndex) const;
    var pop();
    var& push(var iVar);
    var& insert(int iIndex, var iVar);
    var remove(int iIndex);
    var& unshift(var iVar) { return insert(0, iVar); };
    var shift() { return remove(0); };
    var sort() const;
    var index(var iVar) const;
    var& clear();
    var& presize(int iSize);

    // Allow stream output
    friend std::ostream& operator <<(
        std::ostream& iStream, const var& iVar
    );

    // math.cpp
    var sin() const;
    var cos() const;

    // string.cpp
    var& getline(std::istream& iStream);
    var split(const char* iStr, int iMax=0) const;
    var join(const char* iStr) const;
    
private:

    union dataType {
        char c;
        int i;
        long l;
        float f;
        double d;
        varheap* hp;
    };

    // Putting the data first makes sure it will be aligned on an
    // allocation boundary
    dataType mData; ///< The payload
    int mIndex;     ///< 0, or offset if it's a reference
    dataEnum mType; ///< The data type

    // Methods
    template<class T> T& ref(int iIndex);
    void resize(int iSize);
    int attach();
    int detach(varheap* iData=0);
    const char* typeOf();
    bool reference() const;
    var& dereference();
    int binary(var iData) const;
};

std::ostream& operator <<(std::ostream& iStream, const var& iVar);

#endif // VAR_H