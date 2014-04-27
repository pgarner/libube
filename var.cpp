/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2013
 */

#include <cassert>
#include <cstring>
#include <cstdarg>
#include <stdexcept>

#include "var.h"
#include "varheap.h"

#ifdef VARBOSE
# include <cstdlib>
# define VDEBUG(a) a
#else
# define VDEBUG(a)
#endif


/**
 * The nil var
 *
 * Designed never to be touched, except when something needs to return
 * a reference to nil, or also to clear things.
 */
const var var::nil;

/*
 * Main data accessor
 */
template<> char var::get<char>() const {
    var v(*this); return v.mData.c;
}
template<> int var::get<int>() const {
    var v(*this); return v.mData.i;
}
template<> long var::get<long>() const {
    var v(*this); return v.mData.l;
}
template<> float var::get<float>() const {
    var v(*this); return v.mData.f;
}
template<> double var::get<double>() const {
    var v(*this); return v.mData.d;
}
template<> cfloat var::get<cfloat>() const {
    var v(*this); return v.mData.cf;
}
template<> cdouble var::get<cdouble>() const {
    var v(*this); return *v.mData.hp->ptr<cdouble>(0);
}


template<> char* var::ptr<char>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.c : mData.hp->ptr<char>(-mIndex-1);
    }
    return heap() ? heap()->ptr<char>() : &mData.c;
}
template<> int* var::ptr<int>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.i : mData.hp->ptr<int>(-mIndex-1);
    }
    return heap() ? heap()->ptr<int>() : &mData.i;
}
template<> long* var::ptr<long>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.l : mData.hp->ptr<long>(-mIndex-1);
    }
    return heap() ? heap()->ptr<long>() : &mData.l;
}
template<> float* var::ptr<float>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.f : mData.hp->ptr<float>(-mIndex-1);
    }
    return heap() ? heap()->ptr<float>() : &mData.f;
}
template<> double* var::ptr<double>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.d : mData.hp->ptr<double>(-mIndex-1);
    }
    return heap() ? heap()->ptr<double>() : &mData.d;
}
template<> cfloat* var::ptr<cfloat>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        return s ? &r.mData.cf : mData.hp->ptr<cfloat>(-mIndex-1);
    }
    return heap() ? heap()->ptr<cfloat>() : &mData.cf;
}
template<> cdouble* var::ptr<cdouble>()
{
    if (reference())
    {
        bool s;
        var& r = varderef(s);
        assert(!s);
        return s
            ? r.mData.hp->ptr<cdouble>(0) : mData.hp->ptr<cdouble>(-mIndex-1);
    }
    return mData.hp->ptr<cdouble>(0);
}


/**
 * Get a string
 *
 * Returns a pointer to the internal string assuming it's a char array.
 */
const char* var::str()
{
    // Normally would be inline, but needs to be defined after the ptr
    // templates.
    return ptr<char>();
}


template<> char var::cast<char>() const {
    return castChar(*this).get<char>();
}
template<> int var::cast<int>() const {
    return castInt(*this).get<int>();
}
template<> long var::cast<long>() const {
    return castLong(*this).get<long>();
}
template<> float var::cast<float>() const {
    return castFloat(*this).get<float>();
}
template<> double var::cast<double>() const {
    return castDouble(*this).get<double>();
}
template<> cfloat var::cast<cfloat>() const {
    return castCFloat(*this).get<cfloat>();
}
template<> cdouble var::cast<cdouble>() const {
    return castCDouble(*this).get<cdouble>();
}


/*
 * Notes
 *
 * main(argc, argv): template for parameters size + array
 *                   (i.e., it's coded into the standard, size is mandatory)
 *
 * set(var, index=-1): template for parameters array + index
 *                     (index is optional, hence last)
 *
 * We want to be able to say
 *
 *  x["Blue"] = 7;
 *
 * This will result in a reference where "Blue" is lost, replaced by a
 * (negative) index.  Hence, operator[] must allocate the "Blue" entry
 * before returning.
 */


/**
 * Default constructor.  Should be all zero.
 */
var::var()
{
    VDEBUG(std::cout << "Ctor(default)" << std::endl);
    mData.hp = 0;
    mIndex = 0;
    mType = TYPE_ARRAY;
}


/**
 * Destructor.  Detaches from heap storage, which in turn reduces the
 * reference count.
 */
var::~var()
{
    VDEBUG(std::cout << "Dtor: ");
    VDEBUG(std::cout << typeOf(mType) << "[" << size() << "]");
    VDEBUG(std::cout << std::endl);
    detach();
}


/**
 * Copy constructor.
 *
 * This one gets called when vars get passed as function parameters
 * and the like, but *not* when vars are returned from functions;
 * that's the move constructor.  Nevertheless, we can't rely on this
 * being called all the time because of copy elision.
 */
var::var(const var& iVar)
{
    VDEBUG(std::cout << "Const copy" << std::endl);
    mData = iVar.mData;
    mIndex = iVar.mIndex;
    mType = iVar.mType;
    attach();
    dereference();
}


/**
 * Copy assignment
 *
 * This is the one that gets called when functions return vars and
 * they are assigned to other vars.  Pass by value just allows
 * in-place dereference.
 */
var& var::operator =(var iVar)
{
    VDEBUG(std::cout << "Copy assignment" << std::endl);

    if (reference())
    {
        // We are a reference; have to write directly into a typed
        // array.  However, no new storage is allocated, so no need to
        // detach from old storage
        int index = -mIndex-1;
        switch (mData.hp->type())
        {
        case TYPE_CHAR:
            *mData.hp->ptr<char>(index) = iVar.cast<char>();
            break;
        case TYPE_INT:
            *mData.hp->ptr<int>(index) = iVar.cast<int>();
            break;
        case TYPE_LONG:
            *mData.hp->ptr<long>(index) = iVar.cast<long>();
            break;
        case TYPE_FLOAT:
            *mData.hp->ptr<float>(index) = iVar.cast<float>();
            break;
        case TYPE_DOUBLE:
            *mData.hp->ptr<double>(index) = iVar.cast<double>();
            break;
        case TYPE_CFLOAT:
            *mData.hp->ptr<cfloat>(index) = iVar.cast<cfloat>();
            break;
        case TYPE_CDOUBLE:
            *mData.hp->ptr<cdouble>(index) = iVar.cast<cdouble>();
            break;
        case TYPE_VAR:
            *mData.hp->ptr<var>(index) = iVar;
            break;
        case TYPE_PAIR:
            mData.hp->ptr<pair>(index)->val = iVar;
            break;
        default:
            throw std::runtime_error("var::operator =(): Unknown type");
        }
    }
    else if ((mType == TYPE_ARRAY) &&
             mData.hp && mData.hp->copyable(iVar.heap()))
    {
        // Not a reference, but we are a copyable view.
        // Could this use varheap::copy() instead?
        mData.hp->set(iVar, 0);
    }
    else
    {
        // Not a reference.  Record whether the old value was a heap
        // allocation. If so, detach it after attaching the new value.
        varheap* tmp = ((mType == TYPE_ARRAY) && mData.hp) ? mData.hp : 0;
        mData = iVar.mData;
        mIndex = iVar.mIndex;
        mType = iVar.mType;
        attach();
        if (tmp)
            detach(tmp);
        dereference();
    }
    return *this;
}


/**
 * Move constructor
 *
 * This takes care of functions returning var.  It cannot dereference.
 * For move assigment, just assume copy assignment is enough.
 */
var::var(var&& iVar)
{
    VDEBUG(std::cout << "Move" << std::endl);

    // Move the content to the new var
    mData = iVar.mData;
    mIndex = iVar.mIndex;
    mType = iVar.mType;

    // Set the old one to nil.  ...although doing that via operator=()
    // causes trouble
    iVar.mData.hp = 0;
    iVar.mIndex = 0;
    iVar.mType = TYPE_ARRAY;
}


/**
 * Constructors that take an initialiser.  These are the primary
 * (only?) means of converting the builtin types to var.
 */

var::var(char iData)
{
    VDEBUG(std::cout << "Ctor(char): " << iData << std::endl);
    mData.c = iData;
    mIndex = 0;
    mType = TYPE_CHAR;
}

var::var(int iData)
{
    VDEBUG(std::cout << "Ctor(int): " << iData << std::endl);
    mData.i = iData;
    mIndex = 0;
    mType = TYPE_INT;
}

var::var(long iData)
{
    VDEBUG(std::cout << "Ctor(long): " << iData << std::endl);
    mData.l = iData;
    mIndex = 0;
    mType = TYPE_LONG;
}

var::var(float iData)
{
    VDEBUG(std::cout << "Ctor(float): " << iData << std::endl);
    mData.f = iData;
    mIndex = 0;
    mType = TYPE_FLOAT;
}

var::var(double iData)
{
    VDEBUG(std::cout << "Ctor(double): " << iData << std::endl);
    mData.d = iData;
    mIndex = 0;
    mType = TYPE_DOUBLE;
}

var::var(cfloat iData)
{
    mData.cf = iData;
    mIndex = 0;
    mType = TYPE_CFLOAT;
}

var::var(cdouble iData) : var()
{
    // cdouble is always heap-only storage
    attach(new varheap(1, &iData));
}

var::var(int iSize, const char* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char*): " << iData << std::endl);
    attach(new varheap(iSize, iData));
}

var::var(const char* iData)
    : var(std::strlen(iData), iData)
{
    // It's all in the init
}

var::var(int iSize, const char* const* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char**): " << iData[0] << std::endl);
    for (int i=0; i<iSize; i++)
        push(iData[i]);
}


var::var(const char* const* iData) : var()
{
    int size = -1;
    while (iData[++size]) {}
    for (int i=0; i<size; i++)
        push(iData[i]);
}


var::var(int iSize, const int* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(int*): " << iData << std::endl);
    attach(new varheap(iSize, iData));
}


var::var(int iSize, var iVar) : var()
{
    if (iVar.type() == TYPE_ARRAY)
        throw std::runtime_error("initialise ctor: cannot init from array");
    *this = iVar;
    resize(iSize);
    for (int i=1; i<iSize; i++)
        at(i) = iVar;
}


/**
 * var dereference
 *
 * When dereferencing, the result can be two kinds of thing.  One is a
 * simple reference into an array.  The other is a var that is part of
 * a var/pair array.  This handles the latter case.  Whacko interface
 * because it has to return a reference to something that already
 * exists, and some success metric.  ...and I've overloaded &var, so
 * &var == this won't work :-(
 */
var& var::varderef(bool& oSuccess)
{
    oSuccess = true;
    int index = -mIndex-1;
    dataEnum type = mData.hp->type();
    if (type == TYPE_VAR)
    {
        return *mData.hp->ptr<var>(index);
    }
    if (type == TYPE_PAIR)
    {
        return mData.hp->ptr<pair>(index)->val;
    }
    oSuccess = false;
    return *this;
}


/**
 * Negation operator
 *
 * It turns out that the negation is easier to code than the equality;
 * i.e., there are lots of not equal cases where you just want to bail
 * out as soon as you know it's not equal.  So the equality operator
 * calls "not this".
 */
bool var::operator !=(var iVar) const
{
    if (type() != iVar.type())
        return true;
    if (size() != iVar.size())
        return true;
    switch (type())
    {
    case TYPE_ARRAY:
        return heap()->neq(iVar.heap());
    case TYPE_CHAR:
        return (get<char>() != iVar.get<char>());
    case TYPE_INT:
        return (get<int>() != iVar.get<int>());
    case TYPE_LONG:
        return (get<long>() != iVar.get<long>());
    case TYPE_FLOAT:
        return (get<float>() != iVar.get<float>());
    case TYPE_DOUBLE:
        return (get<double>() != iVar.get<double>());
    case TYPE_CFLOAT:
        return (get<cfloat>() != iVar.get<cfloat>());
    case TYPE_CDOUBLE:
        return (get<cdouble>() != iVar.get<cdouble>());
    default:
        throw std::runtime_error("operator !=(): Unknown type");
    }

    assert(0);
    return false;
}


/*
 * Operator <
 *
 * This is the one that gets used by std::map in its search.
 */
bool var::operator <(var iVar) const
{
    if (type() != iVar.type())
        return (type() < iVar.type());

    switch (type())
    {
    case TYPE_ARRAY:
        return heap()->lt(iVar.heap());
    case TYPE_CHAR:
        return (get<char>() < iVar.get<char>());
    case TYPE_INT:
        return (get<int>() < iVar.get<int>());
    case TYPE_LONG:
        return (get<long>() < iVar.get<long>());
    case TYPE_FLOAT:
        return (get<float>() < iVar.get<float>());
    case TYPE_DOUBLE:
        return (get<double>() < iVar.get<double>());
    case TYPE_CFLOAT:
        return (std::abs(get<cfloat>()) < std::abs(iVar.get<cfloat>()));
    case TYPE_CDOUBLE:
        return (std::abs(get<cdouble>()) < std::abs(iVar.get<cdouble>()));
    default:
        throw std::runtime_error("operator <(): Unknown type");
    }

    assert(0);
    return false;
}


/**
 * Shallow copy
 *
 * It's shallow in that a new array is created, but if that array
 * itself points to other arrays, they are not duplicated.
 */
var var::copy(bool iAllocOnly) const
{
    // Deref and return if there's no heap
    if (!heap())
        return *this;

    // It's a heap
    var r;
    r.attach(new varheap(*heap(), iAllocOnly));
    return r;
}


bool var::defined() const
{
    // = not undefined
    var v(*this);
    return !( (v.mType == TYPE_ARRAY) && !v.mData.hp );
}


int var::size() const
{
    if (type() == TYPE_ARRAY)
        return heap() ? heap()->size() : 0;
    return 1;
}


var::dataEnum var::type() const
{
    var v (*this);
    return v.mType;
}


template <class T>
bool Cast<T>::sameType(var iVar) const
{
    const T t = 0;
    static var v = t;
    if (v.type() == iVar.type())
        return true;
    return false;
}

template <class T>
var Cast<T>::operator ()(var iVar) const
{
    // Return immediately if no cast is required
    if (sameType(iVar))
        return iVar;

    // Allocate storage and pass it to the other operator
    var r = iVar.copy(true);
    return operator()(iVar, r);
}

template <class T>
var& Cast<T>::operator ()(const var& iVar, var& oVar) const
{
    // Return immediately if no cast is required
    if (sameType(iVar))
    {
        oVar = iVar;
        return oVar;
    }

    // Strings get converted
    if (iVar.heap() && (iVar.heap()->type() == var::TYPE_CHAR))
    {
        oVar = static_cast<T>(iVar.heap()->strtold());
        return oVar;
    }

    // Everything else gets cast<>ed
    switch (iVar.type())
    {
    case var::TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case var::TYPE_CHAR:
        oVar = static_cast<T>(iVar.get<char>());
        break;
    case var::TYPE_INT:
        oVar = static_cast<T>(iVar.get<int>());
        break;
    case var::TYPE_LONG:
        oVar = static_cast<T>(iVar.get<long>());
        break;
    case var::TYPE_FLOAT:
        oVar = static_cast<T>(iVar.get<float>());
        break;
    case var::TYPE_DOUBLE:
        oVar = static_cast<T>(iVar.get<double>());
        break;
    case var::TYPE_CFLOAT:
        oVar = static_cast<T>(iVar.get<cfloat>().real());
        break;
    case var::TYPE_CDOUBLE:
        oVar = static_cast<T>(iVar.get<cdouble>().real());
        break;
    default:
        throw std::runtime_error("Cast::operator(): Unknown type");
    }

    return oVar;
}

Cast<char> var::castChar;
Cast<int> var::castInt;
Cast<long> var::castLong;
Cast<float> var::castFloat;
Cast<double> var::castDouble;
Cast<cfloat> var::castCFloat;
Cast<cdouble> var::castCDouble;

#if 0
var& var::operator +=(var iVar)
{
    switch (type())
    {
    case TYPE_ARRAY:
        if (heap() && (heap()->type() == TYPE_CHAR))
            append(iVar);
        else
            broadcast(iVar, &var::operator +=, &varheap::add);
        break;
    case TYPE_CHAR:
        *ptr<char>() += iVar.cast<char>();
        break;
    case TYPE_INT:
        *ptr<int>() += iVar.cast<int>();
        break;
    case TYPE_LONG:
        *ptr<long>() += iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        *ptr<float>() += iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        *ptr<double>() += iVar.cast<double>();
        break;
    case TYPE_CFLOAT:
        *ptr<cfloat>() += iVar.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *ptr<cdouble>() += iVar.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("operator +=(): Unknown type");
    }

    return *this;
}

var& var::operator -=(var iVar)
{
    switch (type())
    {
    case TYPE_ARRAY:
        broadcast(iVar, &var::operator -=, &varheap::sub);
        break;
    case TYPE_CHAR:
        *ptr<char>() -= iVar.cast<char>();
        break;
    case TYPE_INT:
        *ptr<int>() -= iVar.cast<int>();
        break;
    case TYPE_LONG:
        *ptr<long>() -= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        *ptr<float>() -= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        *ptr<double>() -= iVar.cast<double>();
        break;
    case TYPE_CFLOAT:
        *ptr<cfloat>() -= iVar.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *ptr<cdouble>() -= iVar.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("operator -=(): Unknown type");
    }

    return *this;
}
#endif


var& var::operator *=(var iVar)
{
    switch (type())
    {
    case TYPE_ARRAY:
        broadcast(iVar, &var::operator *=, &varheap::mul);
        break;
    case TYPE_CHAR:
        *ptr<char>() *= iVar.cast<char>();
        break;
    case TYPE_INT:
        *ptr<int>() *= iVar.cast<int>();
        break;
    case TYPE_LONG:
        *ptr<long>() *= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        *ptr<float>() *= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        *ptr<double>() *= iVar.cast<double>();
        break;
    case TYPE_CFLOAT:
        *ptr<cfloat>() *= iVar.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *ptr<cdouble>() *= iVar.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("operator *=(): Unknown type");
    }

    return *this;
}


var& var::operator /=(var iVar)
{
    switch (type())
    {
    case TYPE_ARRAY:
        broadcast(iVar, &var::operator /=);
        break;
    case TYPE_CHAR:
        *ptr<char>() /= iVar.cast<char>();
        break;
    case TYPE_INT:
        *ptr<int>() /= iVar.cast<int>();
        break;
    case TYPE_LONG:
        *ptr<long>() /= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        *ptr<float>() /= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        *ptr<double>() /= iVar.cast<double>();
        break;
    case TYPE_CFLOAT:
        *ptr<cfloat>() /= iVar.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *ptr<cdouble>() /= iVar.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("operator /=(): Unknown type");
    }

    return *this;
}

#if 0
var var::operator +(var iVar) const
{
    var r = copy();
    r += iVar;
    return r;
}

var var::operator -(var iVar) const
{
    var r = copy();
    r -= iVar;
    return r;
}
#endif

var var::operator *(var iVar) const
{
    var r = copy();
    r *= iVar;
    return r;
}

var var::operator /(var iVar) const
{
    var r = copy();
    r /= iVar;
    return r;
}


/**
 * operator[int]
 *
 * In principle, just creates a reference.  In practice it calls
 * resize(), that in turn can create a larger array, or an array of
 * vars if unset.
 *
 * Can't be const because it calls resize(), which might change the
 * type.
 */
var var::operator [](int iIndex)
{
    var& v = dereference();
    if (iIndex < 0)
        throw std::runtime_error("operator [int]: Negative index");
    if (iIndex >= v.size())
        v.resize(iIndex+1);
    return v.reference(iIndex);
}


/**
 * operator[var]
 *
 * In principle, just creates a reference.  However, it is also the
 * primary means of creating a map type (array of TYPE_PAIR).
 *
 * Can't be const because it calls resize(), which might change the
 * type.
 */
var var::operator [](var iVar)
{
    var& v = dereference();
    if (!v)
    {
        // A kind of constructor
        v.attach(new varheap(0, TYPE_PAIR));
    }
    else
        if (v.heap() && (v.heap()->type() != TYPE_PAIR))
            throw std::runtime_error("operator [var]: Not a map");

    int index = v.binary(iVar);
    if ( (index >= v.size()) || (v.heap()->key(index) != iVar) )
        v.insert(iVar, index);
    return v.reference(index);
}


/**
 * The view indexer
 *
 * returns a reference in the same sense as operator [].
 */
var var::operator ()(int iFirst, ...)
{
    // Can't dereference with a va_list, so first convert to index
    va_list ap;
    va_start(ap, iFirst);
    int p = iFirst * stride(0);
    bounds(0, iFirst);
    for (int i=1; i<dim(); i++)
    {
        int d = va_arg(ap, int);
        bounds(i, d);
        p += d * stride(i);
    }
    va_end(ap);
    return at(p);
}


/*
 * Array indirection
 *
 * The main difference between at() and operator[] is that at() is
 * const.  That follows from its being unable to change the array
 * size.
 */
var var::at(int iIndex) const
{
    var v = *this;
    if (!v)
        throw std::runtime_error("var::at(): uninitialised");
    if (v.type() == TYPE_ARRAY)
        return v.reference(iIndex);
    if (iIndex == 0)
        return v;
    throw std::runtime_error("var::at(): Index out of bounds");
}


/*
 * Array indirection
 *
 * The main difference between at() and operator[] is that at() is
 * const.  That follows from it's being unable to change the array
 * size.  As above, but for map type.
 */
var var::at(var iVar) const
{
    if (!defined())
        throw std::runtime_error("var::at(): uninitialised");
    else
        if (heap() && (heap()->type() != TYPE_PAIR))
            throw std::runtime_error("operator [var]: Not a map");

    int index = binary(iVar);
    if ( (index >= size()) || (heap()->key(index) != iVar) )
        return nil;
    return reference(index);
}


/**
 * Never indent a basic var, but do pass the current level along to
 * the array formatter.
 */
void var::format(std::ostream& iStream, int iIndent) const
{
    switch (type())
    {
    case var::TYPE_ARRAY:
        if (heap())
            heap()->format(iStream, iIndent);
        else
            iStream << "nil";
        break;
    case var::TYPE_CHAR:
        iStream << "\'";
        iStream << get<char>();
        iStream << "\'";
        break;
    case var::TYPE_INT:
        iStream << get<int>();
        break;
    case var::TYPE_LONG:
        iStream << get<long>();
        break;
    case var::TYPE_FLOAT:
        iStream << get<float>();
        break;
    case var::TYPE_DOUBLE:
        iStream << get<double>();
        break;
    case var::TYPE_CFLOAT:
        iStream << get<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        //iStream << get<cdouble>();
        throw std::runtime_error("var::format(): cdouble should be array");
        break;
    default:
        throw std::runtime_error("var::format(): Unknown type");
    }
}


/**
 * Generate a range of indeces.  Does pretty much what python's
 * range() does, in that the higher value is not included, and it
 * starts at 0 if the lower value is omitted.  To start at 1 and
 * include the higher value, use range().
 */
var var::irange(var iLo, var iHi, var iStep)
{
    var r;
    while (iLo < iHi)
    {
        r.push(iLo);
        iLo += iStep;
    }
    return r;
}


/**
 * Generate a range.  This one includes both the high and low
 * extremes, and the low extreme defaults to 1, so range(4) gives
 * {1,2,3,4} as you might expect.  This is not what python does for
 * the same argument; if you want that then use irange() which gives
 * {0,1,2,3}.  Note that in python, range() is more of a loop iterator
 * so you tend to want {0,1,2,3}; in C++, loops use the traditional
 * for (i=0; i<N, i++) syntax, so irange() is less useful.
 */
var var::range(var iLo, var iHi, var iStep)
{
    var r;
    while (iLo <= iHi)
    {
        r.push(iLo);
        iLo += iStep;
    }
    return r;
}


std::ostream& operator <<(std::ostream& iStream, var iVar)
{
    iVar.format(iStream);
    return iStream;
}


var& var::resize(int iSize)
{
    VDEBUG(std::cout << "var::resize(" << iSize << ")" << std::endl);
    assert(iSize >= 0);
    var& v = dereference();
    if (!v.heap())
    {
        // Not allocated
        if (((v.type() != TYPE_ARRAY) && (iSize > 1)) ||
             (v.type() == TYPE_ARRAY))
        {
            // Need to allocate
            if (!v)
            {
                v.attach(new varheap(iSize, v.mType));
            }
            else
            {
                var tmp = v;
                v.mType = TYPE_ARRAY;
                v.attach(new varheap(iSize, tmp.mType));
                v.at(0) = tmp;
            }
        }
    }
    else
    {
        // It's allocated already
        heap()->resize(iSize);
    }
    return *this;
}


/**
 * Attaches a var to a varheap.
 */
int var::attach(varheap* iHeap)
{
    if (iHeap)
    {
        mData.hp = iHeap;
        return mData.hp->attach();
    }
    if ((mType == TYPE_ARRAY) && mData.hp)
        return mData.hp->attach();
    return 0;
}


/**
 * Detaches a var from a varheap.  If a varheap is passed as argument,
 * detaches from that varheap, otherwise detaches from the current one
 * if it exists.
 */
int var::detach(varheap* iHeap)
{
    if (iHeap)
        return iHeap->detach();
    if ((mType == TYPE_ARRAY) && mData.hp)
        return mData.hp->detach();
    return 0;
}


const char* var::typeOf(dataEnum iType)
{
    switch (iType)
    {
    case TYPE_ARRAY: return "array";
    case TYPE_CHAR: return "char";
    case TYPE_INT: return "int";
    case TYPE_LONG: return "long";
    case TYPE_FLOAT: return "float";
    case TYPE_DOUBLE: return "double";
    case TYPE_CFLOAT: return "cfloat";
    case TYPE_CDOUBLE: return "cdouble";
    case TYPE_VAR: return "var";
    case TYPE_PAIR: return "pair";
    default:
        throw std::runtime_error("typeOf(): Unknown type");
    }
}


var var::typeOf()
{
    if (mType == TYPE_ARRAY)
    {
        var s = "array[";
        s += typeOf(heap()->type());
        s += "]";
        return s;
    }
    return typeOf(mType);
}


/**
 * Push a value onto a stack
 *
 * Push is the fundamental way of building arrays, combined with the
 * way resize() works.  Pushing something of an array type creates an
 * array of var; pushing something of a builtin type creates a dense
 * array of that type.
 */
var& var::push(var iVar)
{
    VDEBUG(std::cout << "push: ");
    VDEBUG(std::cout << iVar.typeOf() << " " << typeOf());
    VDEBUG(std::cout << std::endl);
    if (!defined())
    {
        // Uninitialised
        if (iVar.type() != TYPE_ARRAY)
        {
            *this = iVar;
            return *this;
        }
    }
    else
    {
        // Already initialised
        if ( (type() != TYPE_ARRAY) && (iVar.type() != type()) )
            throw std::runtime_error("push(): Incompatible types");
    }

    (*this)[size()] = iVar;
    return *this;
}


var var::pop()
{
    var r = at(size()-1);
    resize(size()-1);
    return r;
}


/**
 * Insert.  Not a fundamentally efficient thing for an array, and not
 * implemented in an efficient way.
 */
var& var::insert(var iVar, int iIndex)
{
    if (iIndex > size())
        throw std::range_error("insert(): index too large");

    if (heap() && (heap()->type() == TYPE_VAR))
    {
        // Implies array; insert a single var
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
            at(i) = at(i-1);
        at(iIndex) = iVar;
    }
    else if (heap() && (heap()->type() == TYPE_PAIR))
    {
        // Implies array; insert a single pair
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
        {
            heap()->key(i) = heap()->key(i-1);
            at(i) = at(i-1);
        }
        heap()->key(iIndex) = iVar;
        at(iIndex).clear();
    }
    else
    {
        // It's a fundamental type, insert the whole array
        resize(size()+iVar.size());
        for (int i=size()-1; i>iIndex+iVar.size()-1; i--)
            at(i) = at(i-iVar.size());
        for (int i=0; i<iVar.size(); i++)
            at(iIndex+i) = iVar[i];
    }

    return *this;
}


var var::remove(int iIndex)
{
    assert(iIndex >= 0);
    if (iIndex > size()-1)
        throw std::range_error("remove(): index too large");

    var r = at(iIndex).copy();
    for (int i=iIndex+1; i<size(); i++)
        at(i-1) = at(i);
    resize(size()-1);
    return r;
}


var var::sort() const
{
    var r;
    r.presize(size());
    for (int i=0; i<size(); i++)
    {
        int p = r.binary(at(i));
        r.insert(at(i), p);
    }
    return r;
}


var var::index(var iVar) const
{
    var r; // Undefined
    for (int i=0; i<size(); i++)
        if (at(i) == iVar)
            r = i;
    return r;
}


int var::index() const
{
    if (!defined())
        throw std::runtime_error("var::index(): undefined");
    if (!reference())
        throw std::runtime_error("var::index(): not a reference");
    int i = -mIndex-1;
    return i;
}


/**
 * Sets the value to the equivalent of undefined
 */
var& var::clear()
{
    *this = nil;
    return *this;
}


var& var::presize(int iSize)
{
    // There is no doubt a more efficient way
    int s = size();
    resize(iSize);
    resize(s);
    return *this;
}


var var::sum() const
{
    var sum = 0.0;
    for (int i=0; i<size(); i++)
        sum += at(i);
    return sum;
}


/* Quite likely to be unstable in general */
var var::prod() const
{
    var prod = 1.0;
    for (int i=0; i<size(); i++)
        prod *= at(i);
    return prod;
}


/**
 * Usable both as a boolean (if this var uses a heap allocation) and
 * as a means to get the pointer.
 */
varheap* var::heap() const
{
    var v(*this);
    return ( (v.mType == TYPE_ARRAY) && v.mData.hp ) ? v.mData.hp : 0;
}


/**
 * True if this var is a reference
 */
bool var::reference() const
{
    VDEBUG(std::cout << "var::reference(): " << mIndex << std::endl);
    if ((mIndex < 0) && (mType == TYPE_ARRAY))
        return true;
    return false;
}


/**
 * Reference constructor
 *
 * Dereferences iVar, constructing a reference to the resulting array.
 */
var var::reference(int iIndex) const
{
    if (type() != TYPE_ARRAY)
        throw std::runtime_error("var::reference: cannot reference non array");
    var v(*this);
    v.mIndex = -iIndex-1;
    return v;
}


/**
 * Converts a reference to an actual var in place
 *
 * However, if the ref is to another var, it takes a copy but returns
 * a reference to the original var.  That way, it can be modified by
 * the caller if necessary.
 */
var& var::dereference()
{
    if (!reference())
        return *this;

    bool s;
    var& r = varderef(s);
    if (s)
    {
        // Set mIndex to not-a-reference, then let operator=() do the
        // housekeeping
        mIndex = 0;
        *this = r;
        return r;
    }
    else
    {
        // Normal de-reference
        assert(mData.hp);
        int index = -mIndex-1;
        dataEnum type = mData.hp->type();
        varheap* tmp = mData.hp;
        mType = type;
        mIndex = 0;
        switch (type)
        {
        case TYPE_CHAR:
            mData.c = *mData.hp->ptr<char>(index);
            break;
        case TYPE_INT:
            mData.i = *mData.hp->ptr<int>(index);
            break;
        case TYPE_LONG:
            mData.l = *mData.hp->ptr<long>(index);
            break;
        case TYPE_FLOAT:
            mData.f = *mData.hp->ptr<float>(index);
            break;
        case TYPE_DOUBLE:
            mData.d = *mData.hp->ptr<double>(index);
            break;
        case TYPE_CFLOAT:
            mData.cf = *mData.hp->ptr<cfloat>(index) ;
            break;
        case TYPE_CDOUBLE:
            mType = TYPE_ARRAY;
            attach(new varheap(1, mData.hp->ptr<cdouble>(index)));
            break;
        default:
            throw std::runtime_error("var::dereference(): Unknown type");
        }
        detach(tmp);
    }
    return *this;
}


/**
 * Binary search
 */
int var::binary(var iData) const
{
    if (size() == 0)
        return 0;

    int lo = 0;
    int hi = size();
    bool pair =  // index on key rather than value
        (heap() && (heap()->type() == TYPE_PAIR));
    while (lo != hi)
    {
        int pos = (hi-lo)/2 + lo;
        var x = pair ? heap()->key(pos) : at(pos);
        if (x < iData)
            lo = pos+1;
        else
            hi = pos;
    }
    return hi;
}


/**
 * Assuming that *this is an array, returns a view of the array.  A
 * view is just another array, of type int, holding the dimensions of
 * the new view.
 */
var var::view(const std::initializer_list<int> iList, int iOffset)
{
    var v;

    // The first entry is the offset; remember to add on the current
    // offset which could be non-zero if we are already a view
    v.push(iOffset+offset());

    // First entry of each subsequent pair is the dimension
    for (const int* it=begin(iList); it!=end(iList); ++it)
    {
        v.push(*it);
        v.push(0);
    }

    // The second of each pair is the stride
    int p = 1;
    for (int i=iList.size()-1; i>=0; i--)
    {
        v[i*2+2] = p;
        p *= v.heap()->viewRef(i*2+1);
    }

    // the p that drops out should be the overall size
    if (!defined())
        resize(p);
    else
        if (p+iOffset > size())
            throw std::runtime_error("var::view(): Array too small for view");

    // Tell the new heap object that it's a view of this
    assert(mType == TYPE_ARRAY);
    v.heap()->setView(heap());

    return v;
}


int var::dim() const
{
    if (!heap() || !heap()->view())
        // It's not an array, or it's an array but not a view
        return 1;
    return heap()->dim();
}


int var::offset() const
{
    if (!heap() || !heap()->view())
        return 0;
    return heap()->offset();
}


var& var::offset(int iOffset)
{
    if (!heap() || !heap()->view())
        throw std::runtime_error("var::offset(): not a view");
    heap()->offset(iOffset);
    return *this;
}


int var::shape(int iDim) const
{
    if (!heap() || !heap()->view())
    {
        if (iDim > 0)
            throw std::runtime_error("var::shape(): dimension too large");
        else
            return size();
    }
    return heap()->shape(iDim);
}


int var::stride(int iDim) const
{
    if (!heap() || !heap()->view())
        throw std::runtime_error("var::stride(): Not a view");
    return heap()->stride(iDim);
}


void var::bounds(int iDim, int iIndex) const
{
    if (iIndex < 0 || iIndex >= shape(iDim))
        throw std::runtime_error("var::bounds(): index out of bounds");
}


/**
 * Basic constructor
 *
 * Initialises a var of type char as the buffer
 */
varbuf::varbuf()
{
    // Type char; ensure it's on the heap
    mVar = "";
    mVar.presize(2);

    // Set the pointers meaning the buffer has zero size.  This will
    // cause the ostream machinery to call overflow() on every write
    char* str = mVar.ptr<char>();
    setp(str, str);
}


/**
 * Catch buffer overflows
 *
 * In fact, this gets called on every write
 */
varbuf::int_type varbuf::overflow(int_type iInt)
{
    if (iInt != traits_type::eof())
        mVar.push(iInt);
    return iInt;
}


vstream::vstream()
{
    // Tell the base class to use the buffer in the derived class
    rdbuf(&mVarBuf);
}


/**
 * vruntime_error constructor
 *
 * This is really just a runtime_error that takes a var as its
 * argument.  The var is formatted, so you can have arbitrary values
 * appear in the what() string.
 *
 * Two things are wrong here: One is that it should be
 * namespace::runtime_error.  The other is that what() should do the
 * formatting, not the constructor.  That is because memory is more
 * likely to have been cleared by the time what() gets executed.  It's
 * this way because what() is const, and "operator&" and all the
 * things returning strings aren't.
 */
vruntime_error::vruntime_error(var iVar)
{
    vstream vs;
    vs << iVar;
    mVar = vs.var();
    mStr = mVar.ptr<char>();
}

const char* vruntime_error::what() const noexcept
{
    return mStr;
}
