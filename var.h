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
#include <initializer_list>

#include <complex>
typedef std::complex<float> cfloat;
typedef std::complex<double> cdouble;

class varheap;
class var;


/**
 * Unary functor
 *
 * A unary functor just acts on itself.
 */
class UnaryFunctor
{
public:
    UnaryFunctor() { mDim = 0; };
    virtual ~UnaryFunctor() {};
    virtual var operator ()(var iVar) const = 0;
    virtual var& operator ()(const var& iVar, var& oVar) const = 0;
protected:
    int mDim;
    virtual void broadcast(var iVar1, var oVar) const;
    virtual void array(var iVar, int iOffset) const;
};


/**
 * Binary functor
 *
 * A binary functor is a functor of two variables.
 */
class BinaryFunctor
{
public:
    virtual ~BinaryFunctor() {};
    virtual var operator ()(var iVar1, var iVar2) const = 0;
    virtual var& operator ()(const var& iVar1, const var& iVar2, var& oVar)
        const = 0;
protected:
    virtual void broadcast(var iVar1, var iVar2, var oVar) const;
    virtual void array(var iVar1, var iVar2, int iOffset) const;
};


/**
 * Casting functor
 */
template <class T>
class Cast : public UnaryFunctor
{
public:
    bool sameType(var iVar) const;
    var operator ()(var iVar) const;
    var& operator ()(const var& iVar, var& oVar) const;
};


/**
 * Tan functor (exemplar unary functor)
 */
class Tan : public UnaryFunctor
{
public:
    var operator ()(var iVar) const;
    var& operator ()(const var& iVar, var& oVar) const;
};


/**
 * Power functor (exemplar binary functor)
 */
class Pow : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
};


/**
 * Set/Copy functor
 */
class Set : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
protected:
    void array(var iVar1, var iVar2, int iOffset) const;
};


/**
 * Addition functor
 */
class Add : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
protected:
    void array(var iVar1, var iVar2, int iOffset) const;
};


/**
 * Subtraction functor
 */
class Sub : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
protected:
    void array(var iVar1, var iVar2, int iOffset) const;
};


/**
 * Multiplication functor
 */
class Mul : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
protected:
    void broadcast(var iVar1, var iVar2, var oVar) const;
    void array(var iVar1, var iVar2, int iOffset) const;
    void scale(var iVar1, var iVar2, int iOffset) const;
};


/**
 * Division functor
 * There's no array operation.
 */
class Div : public BinaryFunctor
{
public:
    var operator ()(var iVar1, var iVar2) const;
    var& operator ()(const var& iVar1, const var& iVar2, var& oVar) const;
};


/**
 * Absolute sum functor
 */
class ASum : public UnaryFunctor
{
public:
    var operator ()(var iVar) const;
    var& operator ()(const var& iVar, var& oVar) const;
protected:
    void array(var iVar, int iOffset) const;
};


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
        TYPE_CFLOAT,
        TYPE_CDOUBLE,
        TYPE_VAR,
        TYPE_PAIR
    };

    // Statics
    static const var nil;

    // Functors
    static Tan tan;
    static Pow pow;
    static Set set;
    static Add add;
    static Sub sub;
    static Mul mul;
    static Div div;

    // Casting functors
    static Cast<char> castChar;
    static Cast<int> castInt;
    static Cast<long> castLong;
    static Cast<float> castFloat;
    static Cast<double> castDouble;
    static Cast<cfloat> castCFloat;
    static Cast<cdouble> castCDouble;

    // Special member functions
    var();
    ~var();
    var(const var& iVar);
    var& operator =(var iVar);
    var(var&& iVar);

    // Overloaded constructors
    var(char iData);
    var(int iData);
    var(long iData);
    var(float iData);
    var(double iData);
    var(cfloat iData);
    var(cdouble iData);
    var(const char* iData);
    var(int iSize, const char* iData);
    var(int iSize, const char* const* iData);
    var(const char* const* iData);
    var(int iSize, const int* iData);
    var(int iSize, var iVar);

    // Data accessor
    template<class T> T get() const;
    template<class T> T* ptr();
    const char* str();

    // Operators
    bool operator !=(var iVar) const;
    bool operator <(var iVar) const;
    bool operator ==(var iVar) const { return !(*this != iVar); };
    bool operator >(var iVar) const { return iVar < *this; };
    bool operator <=(var iVar) const { return !(*this > iVar); };
    bool operator >=(var iVar) const { return !(*this < iVar); };
    var& operator +=(var iVar) { return add(*this, iVar, *this); };
    var& operator -=(var iVar) { return sub(*this, iVar, *this); };
    var& operator *=(var iVar) { return mul(*this, iVar, *this); };
    var& operator /=(var iVar) { return div(*this, iVar, *this); };
    var operator +(var iVar) const { return add(*this, iVar); };
    var operator -(var iVar) const { return sub(*this, iVar); };
    var operator *(var iVar) const { return mul(*this, iVar); };
    var operator /(var iVar) const { return div(*this, iVar); };
    var operator [](int iIndex);
    var operator [](var iVar);
    var operator ()(int iFirst, ...);
    explicit operator bool() const { return defined(); };
    var operator ,(var iVar) { return push(iVar); };

    // Methods
    template<class T> T cast() const;
    var at(int iIndex) const;
    var at(var iVar) const;
    var copy(bool iAllocOnly=false) const;
    bool defined() const;
    int size() const;
    dataEnum type() const;
    varheap* heap() const;
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
    int index() const;
    var& clear();
    var& resize(int iSize);
    var& presize(int iSize);
    var sum() const;
    var prod() const;
    void format(std::ostream& iStream, int iIndent = 0) const;
    static var range(var iHi) { return range(iHi-iHi+1, iHi); };
    static var range(var iLo, var iHi, var iStep=1);
    static var irange(var iHi) { return irange(iHi-iHi, iHi); };
    static var irange(var iLo, var iHi, var iStep=1);

    // Chums
    friend std::ostream& operator <<(
        std::ostream& iStream, var iVar
    );

    // math.cpp
    var abs() const;
    var floor() const;
    var sin() const;
    var cos() const;
    var sqrt() const;
    var asum() const;

    // string.cpp
    var& getline(std::istream& iStream);
    var split(const char* iStr, int iMax=0) const;
    var join(const char* iStr) const;
    var strip();
    var& sprintf(const char* iFormat, ...);
    bool search(var iRE);
    bool match(var iRE);
    var replace(var iRE, var iStr);

    // Files
    var& read(const char* iFile, const char* iType);
    var& write(const char* iFile, const char* iType);

    // Tensors
    var view(const std::initializer_list<int> iList, int iOffset=0);
    int offset() const;
    var& offset(int iOffset);
    var& advance(int iAdvance) { return offset(offset() + iAdvance); };
    int shape(int iDim) const;
    int stride(int iDim) const;
    void bounds(int iDim, int iIndex) const;
    int dim() const;
    var typeOf();

private:

    union dataType {
        dataType() {}; // Because of cfloat
        char c;
        int i;
        long l;
        float f;
        double d;
        cfloat cf;
        varheap* hp;
    };

    // Putting the data first makes sure it will be aligned on an
    // allocation boundary
    dataType mData; ///< The payload
    int mIndex;     ///< 0, or (negative) offset if it's a reference
    dataEnum mType; ///< The data type

    // Methods
    var& varderef(bool& oSuccess);
    bool reference() const;
    var reference(int iIndex) const;
    var& dereference();
    int attach(varheap* iHeap=0);
    int detach(varheap* iHeap=0);
    const char* typeOf(dataEnum iType);
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
    const char* operator &() { return mVarBuf.var().str(); };

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


/**
 * Abstract class for dynamically loaded file handlers.  Defines the
 * interface that file handlers must implement.
 */
class varfile
{
public:
    virtual ~varfile() {};
    virtual var read(const char* iFile) = 0;
    virtual void write(const char* iFile, var iVar) = 0;
};


extern "C" {
    /**
     * Function with C linkage that must exist in the dynamically
     * loaded library.  It should return a varfile by calling new on
     * the derived class within the library.  It is part of the
     * interface.
     */
    void factory(varfile** oFile);
}


/**
 * Class to dynamically load a file handler
 */
class vfile
{
public:
    vfile(const char* iType="txt");
    ~vfile();
    var read(const char* iFile) {
        return mVarFile->read(iFile);
    };
    void write(const char* iFile, var iVar) {
        return mVarFile->write(iFile, iVar);
    };

private:
    void* mHandle;     ///< Handle for dynamic library
    varfile* mVarFile; ///< Pointer to instance of file handler
};


#endif // VAR_H
