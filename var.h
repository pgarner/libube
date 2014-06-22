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

#include <ind.h>

namespace libvar
{
    typedef std::complex<float> cfloat;
    typedef std::complex<double> cdouble;

    class varheap;
    class var;

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
        virtual var operator ()(const var& iVar, var* oVar=0) const = 0;
    protected:
        int mDim;
        virtual void broadcast(var iVar1, var* oVar) const;
        virtual void array(var iVar, var* oVar, int iIndex) const;
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
        virtual var operator ()(const var& iVar1, const var& iVar2, var* oVar=0)
            const = 0;
    protected:
        virtual void broadcast(var iVar1, var iVar2, var* oVar) const;
        virtual void array(var iVar1, var iVar2, var* oVar, int iOffset) const;
    };


#   define BASIC_UNARY_FUNCTOR_DECL(f)                          \
    class f : public UnaryFunctor                               \
    {                                                           \
    public:                                                     \
        var operator ()(const var& iVar, var* oVar=0) const;    \
    };

#   define BASIC_BINARY_FUNCTOR_DECL(f)                         \
    class f : public BinaryFunctor                              \
    {                                                           \
    public:                                                     \
        var operator ()(                                        \
            const var& iVar1, const var& iVar2, var* oVar=0     \
        ) const;                                                \
    };

    BASIC_UNARY_FUNCTOR_DECL(Abs)
    BASIC_UNARY_FUNCTOR_DECL(Sin)
    BASIC_UNARY_FUNCTOR_DECL(Cos)
    BASIC_UNARY_FUNCTOR_DECL(Tan)
    BASIC_UNARY_FUNCTOR_DECL(Floor)
    BASIC_UNARY_FUNCTOR_DECL(Sqrt)
    BASIC_UNARY_FUNCTOR_DECL(Log)
    BASIC_BINARY_FUNCTOR_DECL(Pow)


    template <class T>
    BASIC_UNARY_FUNCTOR_DECL(Cast)


    /**
     * Set/Copy functor
     */
    class Set : public BinaryFunctor
    {
    public:
        var operator ()(const var& iVar1, const var& iVar2, var* oVar=0) const;
    protected:
        void array(var iVar1, var iVar2, var* oVar, int iOffset) const;
    };


    /**
     * Addition functor
     */
    class Add : public BinaryFunctor
    {
    public:
        var operator ()(const var& iVar1, const var& iVar2, var* oVar=0) const;
    protected:
        void array(var iVar1, var iVar2, var* oVar, int iOffset) const;
    };


    /**
     * Subtraction functor
     */
    class Sub : public BinaryFunctor
    {
    public:
        var operator ()(const var& iVar1, const var& iVar2, var* oVar=0) const;
    protected:
        void array(var iVar1, var iVar2, var* oVar, int iOffset) const;
    };


    /**
     * Multiplication functor
     */
    class Mul : public BinaryFunctor
    {
    public:
        var operator ()(const var& iVar1, const var& iVar2, var* oVar=0) const;
    protected:
        void broadcast(var iVar1, var iVar2, var* oVar) const;
        void array(var iVar1, var iVar2, var* oVar, int iOffset) const;
        void scale(var iVar1, var iVar2, var* oVar, int iOffset) const;
    };


    /**
     * Division functor
     * There's no array operation.
     */
    class Div : public BinaryFunctor
    {
    public:
        var operator ()(const var& iVar1, const var& iVar2, var* oVar=0) const;
    };


    /**
     * Absolute sum functor
     */
    class ASum : public UnaryFunctor
    {
    public:
        ASum() { mDim = 1; };
        var operator ()(const var& iVar, var* oVar=0) const;
    protected:
        void array(var iVar, var* oVar, int iOffset) const;
    };


    /**
     * Basic sum functor
     */
    class Sum : public UnaryFunctor
    {
    public:
        Sum() { mDim = 1; };
        var operator ()(const var& iVar, var* oVar=0) const;
    protected:
        void array(var iVar, var* oVar, int iOffset) const;
    };


    // Statics
    extern const var nil;

    // Casting functors
    extern Cast<char> castChar;
    extern Cast<int> castInt;
    extern Cast<long> castLong;
    extern Cast<float> castFloat;
    extern Cast<double> castDouble;
    extern Cast<cfloat> castCFloat;
    extern Cast<cdouble> castCDouble;

    // stdlib Functors
    extern Abs abs;
    extern Sin sin;
    extern Cos cos;
    extern Tan tan;
    extern Floor floor;
    extern Sqrt sqrt;
    extern Log log;
    extern Pow pow;

    // BLAS functors
    extern Set set;
    extern Add add;
    extern Sub sub;
    extern Mul mul;
    extern Div div;
    extern ASum asum;
    extern Sum sum;


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
        var& operator +=(var iVar) { add(*this, iVar, this); return *this; };
        var& operator -=(var iVar) { sub(*this, iVar, this); return *this; };
        var& operator *=(var iVar) { mul(*this, iVar, this); return *this; };
        var& operator /=(var iVar) { div(*this, iVar, this); return *this; };
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
        bool is(var& iVar) const;
        var at(int iIndex) const;
        var at(var iVar) const;
        var copy(bool iAllocOnly=false) const;
        bool defined() const;
        int size() const;
        ind type() const;
        ind atype() const;
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
        ind index(var iVar) const;
        var& clear();
        var& array();
        var& resize(int iSize);
        var& presize(int iSize);
        void format(std::ostream& iStream, int iIndent = 0) const;
        static var range(var iHi) { return range(iHi-iHi+1, iHi); };
        static var range(var iLo, var iHi, var iStep=1);
        static var irange(var iHi) { return irange(iHi-iHi, iHi); };
        static var irange(var iLo, var iHi, var iStep=1);

        // Methods for functors in math.cpp
        var floor() { return libvar::floor(*this, this); };
        var sin() { return libvar::sin(*this, this); };
        var cos() { return libvar::cos(*this, this); };
        var sqrt() { return libvar::sqrt(*this, this); };
        var log() { return libvar::log(*this, this); };
        var abs() { return libvar::abs(*this, this); };

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
        var view(var iShape, int iOffset=0);
        int offset() const;
        var& offset(int iOffset);
        var& advance(int iAdvance) { return offset(offset() + iAdvance); };
        var shape() const;
        int shape(int iDim) const;
        int stride(int iDim) const;
        void bounds(int iDim, int iIndex) const;
        int dim() const;
        var typeOf();

    private:

        union dataType {
            dataType() { hp = 0; }; // Because of cfloat
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
        ind mType;      ///< The data type or reference index

        // Methods
        var& varderef();
        bool reference() const;
        var reference(int iIndex) const;
        var& dereference();
        int attach(varheap* iHeap=0);
        int detach(varheap* iHeap=0);
        const char* typeOf(ind iType);
        int binary(var iData) const;
        void setStrides(var& iVar, int iSize, int iOffset);
    };


    /*
     * Functions
     */
    std::ostream& operator <<(std::ostream& iStream, var iVar);
    var view(const std::initializer_list<int> iShape, var iType=nil);
    var view(var iShape, var iType=nil);


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
        const char* str() { return mVarBuf.var().str(); };

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


    /**
     * DFT functor
     */
#include <mkl_dfti.h>
    class DFT : public UnaryFunctor
    {
    public:
        DFT(int iSize, var iForwardType=0.0f, bool iInverse=false);
        ~DFT();
        var operator ()(const var& iVar, var* oVar=0) const;
    private:
        void array(var iVar, var* oVar, int iOffset) const;
        DFTI_DESCRIPTOR_HANDLE mHandle;
        var mForwardType;
        var mInverseType;
        int mOSize;
        bool mInverse;
    };
}

#endif // VAR_H
