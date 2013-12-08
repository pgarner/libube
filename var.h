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
#include <stdexcept>

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
        TYPE_VAR,
        TYPE_PAIR
    };

    // Special member functions
    var();
    ~var();
    var(const var& iVar);
    var& operator =(var iVar);

    // Overloaded constructors
    var(char iData);
    var(int iData);
    var(long iData);
    var(float iData);
    var(double iData);
    var(const char* iData);
    var(int iSize, const char* iData);
    var(int iSize, const char* const* iData);
    var(const char* const* iData);
    var(int iSize, const int* iData);
    var(int iSize, int iFirst, ...);
    var(int iSize, float iFirst, ...);

    // Operators
    bool operator ==(var iVar) const;
    bool operator !=(var iVar) const;
    bool operator <(var iVar) const;
    var& operator +=(var iVar);
    var& operator -=(var iVar);
    var& operator *=(var iVar);
    var& operator /=(var iVar);
    var operator +(var iVar) const;
    var operator -(var iVar) const;
    var operator *(var iVar) const;
    var operator /(var iVar) const;
    char* operator &();
    var operator [](int iIndex);
    var operator [](var iVar);
    var operator ()(int iFirst, ...);
    explicit operator bool() const { return defined(); };
    var operator ,(var iVar) { return push(iVar); };

    // Methods
    var at(int iIndex) const;
    var at(var iVar) const;
    var copy() const;
    bool defined() const;
    int size() const;
    dataEnum type() const;
    template<class T> T cast();
    bool heap(int iSize = -1) const;
    var pop();
    var top() { return at(size() - 1); };
    var& push(var iVar);
    var& insert(var iVar, int iIndex=0);
    var remove(int iIndex);
    var& unshift(var iVar) { return insert(iVar); };
    var& append(var iVar) { return insert(iVar, size()); };
    var shift() { return remove(0); };
    var sort() const;
    var index(var iVar) const;
    var& clear();
    var& resize(int iSize);
    var& presize(int iSize);
    var sum() const;
    var prod() const;
    void format(std::ostream& iStream, int iIndent = 0);

    // Chums
    friend var& deref(var iVar);
    friend std::ostream& operator <<(
        std::ostream& iStream, var iVar
    );

    // math.cpp
    var abs() const;
    var floor() const;
    var sin() const;
    var cos() const;
    var sqrt() const;
    var pow(var iPower) const;
    var asum() const;

    // string.cpp
    var& getline(std::istream& iStream);
    var split(const char* iStr, int iMax=0) const;
    var join(const char* iStr) const;
    var& strip();
    var& sprintf(const char* iFormat, ...);

    // Files
    var& read(const char* iFile, const char* iType);
    var& write(const char* iFile, const char* iType);

    // Tensors
    var view(int iDim, int iFirst, ...);
    int shape(int iDim) const;
    int stride(int iDim) const;
    void bounds(int iDim, int iIndex) const;

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
    int mIndex;     ///< 0, or (negative) offset if it's a reference
    dataEnum mType; ///< The data type

    // Methods
    //template<class T> T& ref(int iIndex);
    int attach();
    int detach(varheap* iData=0);
    const char* typeOf(dataEnum iType);
    var typeOf();
    bool reference() const;
    var& dereference();
    int binary(var iData) const;
};


/**
 * A stream buffer that uses a var as the buffer
 */
class varbuf : public std::streambuf
{
public:
    varbuf();
    class var var() const { return mVar; };

private:
    virtual int_type overflow(int_type iInt);

    class var mVar;    ///< The actual buffer
};


/**
 * An ostream that uses a var as the buffer
 */
class vstream : public std::ostream
{
public:
    vstream();
    class var var() const { return mVarBuf.var(); };

private:
    class varbuf mVarBuf;
};


/**
 * Exception class
 */
class vruntime_error : public std::exception
{
public:
    vruntime_error(var iVar);
    virtual const char* what() const noexcept;

private:
    class var mVar;
    char* mStr;
};

#endif // VAR_H
