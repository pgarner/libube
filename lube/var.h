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
#include <chrono>

#include <lube/ind.h>
#include <lube/func.h>
#include <lube/math.h>
#include <lube/string.h>

namespace libube
{
    // Forward declare the heap
    class Heap;

    /**
     * The possible var types
     */
    enum {
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
    extern const var nil;

    // Casting functors
    template <class T>
    BASIC_UNARY_FUNCTOR_DECL(Cast)
    extern Cast<char> castChar;
    extern Cast<int> castInt;
    extern Cast<long> castLong;
    extern Cast<float> castFloat;
    extern Cast<double> castDouble;
    extern Cast<cfloat> castCFloat;
    extern Cast<cdouble> castCDouble;


    /**
     * Class with runtime type determination.
     *
     * The name 'var' is borrowed from ECMAScript.  The syntax is intended to
     * be more from ruby, but we can't use delete() method!() or method?().
     */
    class var
    {
    public:

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
        var(const char* const* iData);
        var(int iSize, const char* iData);
        var(int iSize, const char* const* iData);
        var(int iSize, const int* iData);
        var(int iSize, var iVar);
        var(const std::initializer_list<var> iList);
        var(const std::initializer_list<int> iShape, var iType);

        // Data accessor
        template<class T> T get() const { var v(*this); return v.data<T>(); }
        template<class T> T* ptr(ind iIndex=0);
        const char* str() const;

        // Operators
        bool operator !=(var iVar) const;
        bool operator <(var iVar) const;
        bool operator ==(var iVar) const { return !(*this != iVar); };
        bool operator >(var iVar) const { return iVar < *this; };
        bool operator <=(var iVar) const { return !(*this > iVar); };
        bool operator >=(var iVar) const { return !(*this < iVar); };
        var& operator +=(var iVar) { add(*this, iVar, *this); return *this; };
        var& operator -=(var iVar) { sub(*this, iVar, *this); return *this; };
        var& operator *=(var iVar) { mul(*this, iVar, *this); return *this; };
        var& operator /=(var iVar) { div(*this, iVar, *this); return *this; };
        var operator +(var iVar) const { return add(*this, iVar); };
        var operator -(var iVar) const { return sub(*this, iVar); };
        var operator *(var iVar) const { return mul(*this, iVar); };
        var operator /(var iVar) const { return div(*this, iVar); };
        var operator -() const { return *this * -1; };
        var operator [](int iIndex);
        var operator [](int iIndex) const { return at(iIndex); };
        var operator [](var iVar);
        var operator ()(int iFirst, ...) const;
        explicit operator bool() const { return defined(); };
        var operator ,(var iVar) { return append(iVar); };

        // Methods
        var& dereference();
        template<class T> bool atype() const;
        template<class T> T cast() const;
        bool is(var& iVar) const;
        var at(int iIndex) const;
        var at(var iVar) const;
        var key(int iIndex) const;
        var copy(bool iAllocOnly=false) const;
        bool defined() const;
        int size() const;
        ind type() const;
        ind atype() const;
        var typeStr() const;
        var atypeStr() const;
        Heap* heap() const;
        var pop();
        var top() { return at(size() - 1); };
        var& push(var iVar);
        var& insert(var iVar, int iIndex=0);
        var remove(int iIndex);
        var& unshift(var iVar);
        var& append(var iVar) { return insert(iVar, size()); };
        var shift();
        var sort() const;
        ind index(var iVar) const;
        var& clear();
        var& array();
        var& resize(int iSize);
        var& presize(int iSize);
        void format(std::ostream& iStream, int iIndent = 0) const;

        // Math functors
        var floor() { return libube::floor(*this, *this); };
        var sin() { return libube::sin(*this, *this); };
        var cos() { return libube::cos(*this, *this); };
        var sqrt() { return libube::sqrt(*this, *this); };
        var log() { return libube::log(*this, *this); };
        var exp() { return libube::exp(*this, *this); };
        var pow(var iPow) { return libube::pow(*this, iPow, *this); };
        var real() { return libube::real(*this, *this); };
        var imag() { return libube::imag(*this, *this); };
        var abs() { return libube::abs(*this, *this); };
        var arg() { return libube::arg(*this, *this); };
        var norm() { return libube::norm(*this, *this); };

        // Other functors
        var transpose() { return libube::transpose(*this, *this); };

        // String functors
        ind len();
        var& getline(std::istream& iStream);
        var split(const char* iStr, int iMax=0) const;
        var join(const char* iStr) const;
        var toupper() { return libube::toupper(*this, *this); };
        var tolower() { return libube::tolower(*this, *this); };
        var strip() { return libube::strip(*this, *this); };

        // Regex functors
        var search(var iRE);
        var match(var iRE);
        var replace(var iRE, var iStr);

        // Tensors
        bool view() const;
        var view(const std::initializer_list<int> iList, int iOffset=0);
        var view(var iShape, int iOffset=0);
        var subview(int iDim, ind iOffset);
        int offset() const;
        var& offset(int iOffset);
        var& advance(int iAdvance) { return offset(offset() + iAdvance); };
        var shape() const;
        int shape(int iDim) const;
        int stride(int iDim) const;
        void bounds(int iDim, int iIndex) const;
        int dim() const;

    private:

        union dataType {
            dataType() { hp = 0; }; // Because of cfloat
            char c;
            int i;
            long l;
            float f;
            double d;
            cfloat cf;
            Heap* hp;
        };

        // Putting the data first makes sure it will be aligned on an
        // allocation boundary
        dataType mData; ///< The payload
        ind mType;      ///< The data type or reference index

        // Methods
        template<class T> T& data();
        var& varderef();
        bool reference() const;
        var reference(int iIndex) const;
        int attach(Heap* iHeap=0);
        int detach(Heap* iHeap=0);
        int binary(var iData) const;
        void setStrides(var& iVar, int iSize, int iOffset);
    };


    /*
     * Functions
     */
    std::ostream& operator <<(std::ostream& iStream, var iVar);
    std::istream& operator >>(std::istream& iStream, var& ioVar);
    var view(const std::initializer_list<int> iShape, var iType=nil);
    var view(var iShape, var iType=nil);
    const char* typeStr(ind iType);
    var range(var iLo, var iHi, var iStep=1);
    var range(var iHi);
    var irange(var iLo, var iHi, var iStep=1);
    var irange(var iHi);


    /**
     * A stream buffer to connect a stream to a var
     */
    class varbuf : public std::streambuf
    {
    public:
        varbuf(var iVar);
        explicit operator var () const { return mVar; };
    protected:
        virtual int overflow(int iInt = traits_type::eof());
        virtual int uflow();
        virtual int underflow();
        virtual int pbackfail(int iInt);
        virtual std::streampos seekpos(
            std::streampos iPos, std::ios_base::openmode iMode
        );
    private:
        var mVar; ///< The var being accessed
        int mInd; ///< The get index
    };


    /**
     * An iostream to / from a var
     */
    class varstream : public std::iostream
    {
    public:
        varstream(var iVar = nil);
        operator var() const { return var(mVarBuf); };
        const char* str() { return var(mVarBuf).str(); };
    private:
        class varbuf mVarBuf;
    };


    /**
     * Exception class
     */
    class error : public std::exception
    {
    public:
        error(var iVar);
        virtual const char* what() const noexcept;
    private:
        void backTrace(std::ostream& iStream);
        var mVar;
        const char* mStr;
    };


    /**
     * Timer class
     */
    class timer
    {
    public:
        timer(var iName);
        ~timer();
    private:
        var mName;
        std::chrono::steady_clock::time_point mBegan;
    };

}

#endif // VAR_H
