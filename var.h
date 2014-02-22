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

    // Statics
    static const var nil;

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
    var(const char* iData);
    var(int iSize, const char* iData);
    var(int iSize, const char* const* iData);
    var(const char* const* iData);
    var(int iSize, const int* iData);
    var(int iSize, int iInit);
    var(int iSize, float iInit);
    var(var iVar, int iIndex);

    // Data accessor
    template<class T> T get() const;
    template<class T> T& get();

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
    static var range(var iHi) { return range(iHi-iHi, iHi); };
    static var range(var iLo, var iHi, var iStep=1);

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
    var pow(var iPower) const;
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
    int shape(int iDim) const;
    int stride(int iDim) const;
    void bounds(int iDim, int iIndex) const;
    int dim() const;

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
    var& varderef(bool& oSuccess);
    bool reference() const;
    var& dereference();
    int attach(varheap* iHeap=0);
    int detach(varheap* iHeap=0);
    const char* typeOf(dataEnum iType);
    var typeOf();
    int binary(var iData) const;
    void broadcast(
        var iVar,
        var& (var::*iUnaryOp)(var),
        void (varheap::*iArrayOp)(const varheap*, int, int) = 0
    );
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
    const char* operator &() { return &(mVarBuf.var()); };

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
