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
#ifdef HAVE_BACKTRACE
# include <execinfo.h>
#endif
#include <cxxabi.h>

#include "lube/var.h"
#include "lube/heap.h"
#include "lube/string.h"

#ifdef VARBOSE
# include <cstdlib>
# define VDEBUG(a) a
#else
# define VDEBUG(a)
#endif


using namespace libube;


#define GET(T, P)                                               \
    template<> T libube::var::get<T>() const                    \
    {                                                           \
        if (reference())                                        \
        {                                                       \
            var* v = mData.hp->deref(~mType);                   \
            return v ? v->get<T>() : *mData.hp->ptr##T(~mType); \
        }                                                       \
        return mData.P;                                         \
    }

GET(char, c)
GET(int, i)
GET(long, l)
GET(float, f)
GET(double, d)
GET(cfloat, cf)

template<> cdouble libube::var::get<cdouble>() const
{
    if (reference())
    {
        var* v = mData.hp->deref(~mType);
        return v ? v->get<cdouble>() : *mData.hp->ptrcdouble(~mType);
    }
    return *mData.hp->ptrcdouble(0);
}


namespace libube
{
    /**
     * The nil var
     *
     * Designed never to be touched, except when something needs to return a
     * reference to nil, or also to clear things.
     */
    const var nil;

    /*
     * These are the instantiations of the casters.  They are declared extern
     * in the var class.
     */
    Cast<char> castChar;
    Cast<int> castInt;
    Cast<long> castLong;
    Cast<float> castFloat;
    Cast<double> castDouble;
    Cast<cfloat> castCFloat;
    Cast<cdouble> castCDouble;

    /*
     * Allow data access to be templated
     */
    template<> char& var::data<char>() { return mData.c; };
    template<> int& var::data<int>() { return mData.i; };
    template<> long& var::data<long>() { return mData.l; };
    template<> float& var::data<float>() { return mData.f; };
    template<> double& var::data<double>() { return mData.d; };
    template<> cfloat& var::data<cfloat>() { return mData.cf; };
    template<> cdouble& var::data<cdouble>() {
        // It's too big for a var; always an array
        return *mData.hp->ptrcdouble(0);
    };


    template<> bool var::atype<char>() const {
        return atype() == TYPE_CHAR;
    }
    template<> bool var::atype<int>() const {
        return atype() == TYPE_INT;
    }
    template<> bool var::atype<long>() const {
        return atype() == TYPE_LONG;
    }
    template<> bool var::atype<float>() const {
        return atype() == TYPE_FLOAT;
    }
    template<> bool var::atype<double>() const {
        return atype() == TYPE_DOUBLE;
    }
    template<> bool var::atype<cfloat>() const {
        return atype() == TYPE_CFLOAT;
    }
    template<> bool var::atype<cdouble>() const {
        return atype() == TYPE_CDOUBLE;
    }
    template<> bool var::atype<var>() const {
        return atype() == TYPE_VAR;
    }
    template<> bool var::atype<pair>() const {
        return atype() == TYPE_PAIR;
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
}


/**
 * Pointer data accessor
 *
 * get() returns by value, so there can be no attempt to write to the result.
 * This means they can just dereference.  By contrast, in the case of pointers,
 * we may wish to write to the resulting pointer; this means we can't
 * dereference as the resulting var will be a different copy.
 *
 * The pointer methods are rather basic in that they do not do any type or
 * range checking.  So, if you ask for index 10 of something that is not an
 * array then the index is ignored.  Maybe this will get fixed one day.
 */
#define PTR(T)                                                  \
    template<> T* libube::var::ptr<T>(ind iIndex)               \
    {                                                           \
        if (reference())                                        \
        {                                                       \
            var& r = varderef();                                \
            return (&r != this)                                 \
                ? r.ptr<T>(iIndex) : mData.hp->ptr##T(~mType);  \
        }                                                       \
        return heap() ? heap()->ptr##T(iIndex) : &data<T>();    \
    }

PTR(char)
PTR(int)
PTR(long)
PTR(float)
PTR(double)
PTR(cfloat)
PTR(cdouble)


/**
 * Get a string
 *
 * Returns a pointer to the internal string assuming it's a char array.  The
 * method is const, so it bypasses the ptr<>() methods.
 *
 * Could be inline.
 */
const char* var::str() const
{
    // The static is a cheap trick to avoid thinking about references for the
    // moment
    static char z = 0;
    return heap() ? heap()->ptrchar() : &z;
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
    if (detach() == 0)
    {
        // Some functions, notably shift and unshift, call placement delete.
        // So, the d'tor needs to be robust to multiple calls.
        mData.hp = 0;
        mType = TYPE_ARRAY;
    }
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
    mType = iVar.mType;
    attach();
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
        int index = ~mType;
        switch (mData.hp->type())
        {
        case TYPE_CHAR:
            *mData.hp->ptrchar(index) = iVar.cast<char>();
            break;
        case TYPE_INT:
            *mData.hp->ptrint(index) = iVar.cast<int>();
            break;
        case TYPE_LONG:
            *mData.hp->ptrlong(index) = iVar.cast<long>();
            break;
        case TYPE_FLOAT:
            *mData.hp->ptrfloat(index) = iVar.cast<float>();
            break;
        case TYPE_DOUBLE:
            *mData.hp->ptrdouble(index) = iVar.cast<double>();
            break;
        case TYPE_CFLOAT:
            *mData.hp->ptrcfloat(index) = iVar.cast<cfloat>();
            break;
        case TYPE_CDOUBLE:
            *mData.hp->ptrcdouble(index) = iVar.cast<cdouble>();
            break;
        case TYPE_VAR:
            *mData.hp->ptrvar(index) = iVar;
            break;
        case TYPE_PAIR:
            mData.hp->ptrpair(index)->val = iVar;
            break;
        default:
            throw error("var::operator =(): Unknown type");
        }
    }
    else if ((mType == TYPE_ARRAY) &&
             mData.hp && mData.hp->copyable(iVar.heap()))
    {
        // Not a reference, but we are a copyable view.
        // This is the broadcastable set(); could use Heap::copy() instead?
        set(*this, iVar, *this);
    }
    else
    {
        // Not a reference.  Record whether the old value was a heap
        // allocation. If so, detach it after attaching the new value.
        IHeap* tmp = ((mType == TYPE_ARRAY) && mData.hp) ? mData.hp : 0;
        mData = iVar.mData;
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
    mType = iVar.mType;

    // Set the old one to nil.  ...although doing that via operator=()
    // causes trouble
    iVar.mData.hp = 0;
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
    mType = TYPE_CHAR;
}

var::var(int iData)
{
    VDEBUG(std::cout << "Ctor(int): " << iData << std::endl);
    mData.i = iData;
    mType = TYPE_INT;
}

var::var(long iData)
{
    VDEBUG(std::cout << "Ctor(long): " << iData << std::endl);
    mData.l = iData;
    mType = TYPE_LONG;
}

var::var(float iData)
{
    VDEBUG(std::cout << "Ctor(float): " << iData << std::endl);
    mData.f = iData;
    mType = TYPE_FLOAT;
}

var::var(double iData)
{
    VDEBUG(std::cout << "Ctor(double): " << iData << std::endl);
    mData.d = iData;
    mType = TYPE_DOUBLE;
}

var::var(cfloat iData)
{
    mData.cf = iData;
    mType = TYPE_CFLOAT;
}

var::var(cdouble iData) : var()
{
    // cdouble is always heap-only storage
    attach(new Heap(1, &iData));
}

var::var(const char* iData) : var(std::strlen(iData), iData)
{
    // It's all in the init
}

var::var(const char* const* iData) : var()
{
    int size = -1;
    while (iData[++size]) {}
    for (int i=0; i<size; i++)
        push(iData[i]);
}

var::var(int iSize, const char* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char*): " << iData << std::endl);
    attach(new Heap(iSize, iData));
}

var::var(int iSize, const char* const* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char**): " << iData[0] << std::endl);
    for (int i=0; i<iSize; i++)
        push(iData[i]);
}

var::var(int iSize, const int* iData) : var()
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(int*): " << iData << std::endl);
    attach(new Heap(iSize, iData));
}


var::var(int iSize, var iVar) : var()
{
    if (iVar.type() == TYPE_ARRAY)
        throw error("initialise ctor: cannot init from array");
    *this = iVar;
    resize(iSize);
    for (int i=1; i<iSize; i++)
        at(i) = iVar;
}


var::var(const std::initializer_list<var> iList) : var()
{
    for (const var* it=begin(iList); it!=end(iList); ++it)
        push(*it);
}


var::var(const std::initializer_list<int> iShape, var iType)
{
    *this = libube::view(iShape, iType);
    for (int i=0; i<size(); i++)
        at(i) = iType;
}


/**
 * var dereference
 *
 * When dereferencing, the result can be two kinds of thing.  One is a simple
 * reference into an array.  The other is a var that is part of a var/pair
 * array.  This handles the latter case.
 */
var& var::varderef()
{
    int index = ~mType;
    ind type = mData.hp->type();
    if (type == TYPE_VAR)
    {
        return *mData.hp->ptrvar(index);
    }
    if (type == TYPE_PAIR)
    {
        return mData.hp->ptrpair(index)->val;
    }
    // Return this on "failure"
    return *this;
}


/**
 * Negation operator
 *
 * It turns out that the negation is easier to code than the equality; i.e.,
 * there are lots of not equal cases where you just want to bail out as soon as
 * you know it's not equal.  So the equality operator calls "not this".
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
        throw error("operator !=(): Unknown type");
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
        throw error("operator <(): Unknown type");
    }

    assert(0);
    return false;
}


/**
 * Shallow copy
 *
 * It's shallow in that a new array is created, but if that array itself points
 * to other arrays, they are not duplicated.
 */
var var::copy(bool iAllocOnly) const
{
    // Deref and return if there's no heap
    if (!heap())
        return *this;

    // It's a heap
    var r;
    if (view())
        r.attach(new View(*heap(), iAllocOnly));
    else
        r.attach(new Heap(*heap(), iAllocOnly));
    return r;
}


bool var::defined() const
{
    if (reference())
        return mData.hp->defined(~mType);
    return !( (mType == TYPE_ARRAY) && !mData.hp ); // = not undefined
}


int var::size() const
{
    if (reference())
        return mData.hp->derefSize(~mType);
    if (mType == TYPE_ARRAY)
        return mData.hp ? mData.hp->size() : 0;
    return 1;
}


/**
 * var::type() dereferences but does not indirect to the heap, so it can return
 * TYPE_ARRAY but will never return TYPE_VAR.
 */
ind var::type() const
{
    if (reference())
        return mData.hp->derefType(~mType);
    return mType;
}


/**
 * var::atype() dereferences and indirects to the heap if it exists, so it will
 * never return TYPE_ARRAY, but can return TYPE_VAR.
 */
ind var::atype() const
{
    if (!defined())
        throw error("var::atype(): Undefined");
    if (reference())
        return mData.hp->derefAType(~mType);
    if (mType == TYPE_ARRAY)
        return mData.hp->type();
    return mType;
}


template <class T>
void Cast<T>::scalar(const var& iVar, var& oVar) const
{
    // Return immediately if no cast is required
    const T t = 0;
    static var v = t;
    if (v.type() == iVar.type())
    {
        if (!iVar.is(oVar))
            oVar = iVar;
        return;
    }

    // Strings get converted
    if (iVar.heap() && iVar.atype<char>())
    {
        T t;
        varstream s(iVar);
        s.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        s >> t;
        oVar = t;
        return;
    }

    // Everything else gets cast<>ed
    switch (iVar.type())
    {
    case TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case TYPE_CHAR:
        oVar = static_cast<T>(iVar.get<char>());
        break;
    case TYPE_INT:
        oVar = static_cast<T>(iVar.get<int>());
        break;
    case TYPE_LONG:
        oVar = static_cast<T>(iVar.get<long>());
        break;
    case TYPE_FLOAT:
        oVar = static_cast<T>(iVar.get<float>());
        break;
    case TYPE_DOUBLE:
        oVar = static_cast<T>(iVar.get<double>());
        break;
    case TYPE_CFLOAT:
        oVar = static_cast<T>(iVar.get<cfloat>().real());
        break;
    case TYPE_CDOUBLE:
        oVar = static_cast<T>(iVar.get<cdouble>().real());
        break;
    default:
        throw error("Cast::operator(): Unknown type");
    }
}


/**
 * operator[int]
 *
 * In principle, just creates a reference.  In practice it calls resize(), that
 * in turn can create a larger array, or an array of vars if unset.
 *
 * Can't be const because it calls resize(), which might change the type.
 */
var var::operator [](int iIndex)
{
    if (iIndex < 0)
        throw error("operator [int]: Negative index");
    if (reference())
        return mData.hp->derefInt(~mType, iIndex);
    if (iIndex >= size())
        resize(iIndex+1);
    array();
    return reference(iIndex);
}


/**
 * operator[var]
 *
 * In principle, just creates a reference.  However, it is also the primary
 * means of creating a map type (array of TYPE_PAIR).
 *
 * Simply saying var[nil] will create the pair but return without doing
 * anything.  This means that empty pairs can be created.
 *
 * Can't be const because it calls resize(), which might change the type.
 */
var var::operator [](var iVar)
{
    var& v = dereference();
    if (!v)
    {
        // A kind of constructor
        v.attach(new Heap(0, TYPE_PAIR));
    }
    else
        if (v.heap() && !v.atype<pair>())
            // Fall back to normal array if the var can be cast to int
            return operator [](iVar.cast<int>());
    if (!iVar)
        return nil;
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
var var::operator ()(int iFirst, ...) const
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


/**
 * Check for identical storage.  Returns true if the storage of this and the
 * parameter are refer to the same memory locations.
 */
bool var::is(var& iVar) const
{
    // Are they the same var?
    if (this == &iVar)
        return true;

    // Are they both arrays and point to the same heap?
    IHeap* h1 = heap();
    if (!h1)
        return false;
    IHeap* h2 = iVar.heap();
    if (!h2)
        return false;
    if (h1->ptrchar() == h2->ptrchar())
        return true;
    return false;
}


/*
 * Array indirection
 *
 * The main difference between at() and operator[] is that at() is const.  That
 * follows from its being unable to change the array size.
 */
var var::at(int iIndex) const
{
    var v = *this;
    v.dereference();
    if (!v)
        throw error("var::at(): uninitialised");
    if (v.type() == TYPE_ARRAY)
        return v.reference(iIndex);
    if (iIndex == 0)
        return v;
    throw error("var::at(): Index out of bounds");
}


/*
 * Array indirection
 *
 * The main difference between at() and operator[] is that at() is const.  That
 * follows from it's being unable to change the array size.  As above, but for
 * map type.
 */
var var::at(var iVar) const
{
    if (!defined())
        throw error("var::at(): uninitialised");
    else
        if (heap() && !atype<pair>())
            throw error("operator [var]: Not a map");

    int index = binary(iVar);
    if ( (index >= size()) || (heap()->key(index) != iVar) )
        return nil;
    return reference(index);
}


var var::key(int iIndex) const
{
    if (!atype<pair>())
        throw error("var::key(): Not a map");
    return heap()->key(iIndex);
}


/**
 * Generate a range of indeces.  Does pretty much what python's range() does,
 * in that the higher value is not included, and it starts at 0 if the lower
 * value is omitted.  To start at 1 and include the higher value, use range().
 */
var libube::irange(var iLo, var iHi, var iStep)
{
    var r;
    while (iLo < iHi)
    {
        r.push(iLo);
        iLo += iStep;
    }
    return r;
}


var libube::irange(var iHi)
{
    return libube::irange(iHi-iHi, iHi);
}


/**
 * Generate a range.  This one includes both the high and low extremes, and the
 * low extreme defaults to 1, so range(4) gives {1,2,3,4} as you might expect.
 * This is not what python does for the same argument; if you want that then
 * use irange() which gives {0,1,2,3}.  Note that in python, range() is more of
 * a loop iterator so you tend to want {0,1,2,3}; in C++, loops use the
 * traditional for (i=0; i<N, i++) syntax, so irange() is less useful.
 */
var libube::range(var iLo, var iHi, var iStep)
{
    var r;
    while (iLo <= iHi)
    {
        r.push(iLo);
        iLo += iStep;
    }
    return r;
}


var libube::range(var iHi)
{
    return libube::range(iHi-iHi+1, iHi);
}


var& var::array()
{
    var& v = dereference();
    if (!v.heap())
    {
        var tmp = v;
        v.mType = TYPE_ARRAY;
        v.attach(new Heap(1, tmp.mType));
        v.at(0) = tmp;
    }
    return *this;
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
            (v.type() == TYPE_ARRAY) )
        {
            // Need to allocate
            if (!v)
            {
                v.attach(new Heap(iSize, v.mType));
            }
            else
            {
                var tmp = v;
                v.mType = TYPE_ARRAY;
                v.attach(new Heap(iSize, tmp.mType));
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
 * Attaches a var to a Heap.
 */
int var::attach(IHeap* iHeap)
{
    if (iHeap)
    {
        mData.hp = iHeap;
        return mData.hp->attach();
    }
    if ((mType <= TYPE_ARRAY) && mData.hp)
        return mData.hp->attach();
    return 0;
}


/**
 * Detaches a var from a Heap.  If a Heap is passed as argument, detaches
 * from that Heap, otherwise detaches from the current one if it exists.
 */
int var::detach(IHeap* iHeap)
{
    if (iHeap)
        return iHeap->detach();
    if ((mType <= TYPE_ARRAY) && mData.hp)
        return mData.hp->detach();
    return 0;
}


/**
 * Maps types to strings.  Cannot be a method as it should handle
 * heap()->type() style calls too.
 */
const char* libube::typeStr(ind iType)
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
        throw error("libube::typeStr(): Unknown type");
    }
}


/**
 * typeStr() returns a var being the string representation of the type.  It
 * does not indirect (cf. type()), so the type may be just "array", and will
 * never be "var" or "pair".
 */
var var::typeStr() const
{
    return libube::typeStr(type());
}


/**
 * atypeStr() returns a var being the string representation of the type.  In
 * the case of arrays, it indirects to the heap (cf. atype()), giving a more
 * informative string.
 */
var var::atypeStr() const
{
    if (type() == TYPE_ARRAY)
    {
        varstream s;
        s << "array[" << libube::typeStr(heap()->type()) << "]";
        return var(s);
    }
    return typeStr();
}


/**
 * Append a raw array to an array
 *
 * This is a fundamental array builder.  It's typed so that it doesn't go
 * through the conversion to var process; this in turn allows it to be very
 * fast.
 */
#define APPEND(T)                                               \
    template<> var& var::append<T>(int iSize, const T* iData)   \
    {                                                           \
        array();                                                \
        heap()->append(iSize, iData);                           \
        return *this;                                           \
    }

APPEND(char)
APPEND(int)
APPEND(cdouble)

var& var::append(const char* iStr)
{
    // It should be a special case to avoid the strlen
    array();
    heap()->append(std::strlen(iStr), iStr);
    return *this;
}


/**
 * Push a value onto a stack
 *
 * Push is the fundamental way of building arrays, combined with the way
 * resize() works.  Pushing something of an array type creates an array of var;
 * pushing something of a builtin type creates a dense array of that type.
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
            throw error("push(): Incompatible types");
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

    if (heap() && atype<var>())
    {
        // Implies array; insert a single var
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
            at(i) = at(i-1);
        at(iIndex) = iVar;
    }
    else if (heap() && atype<pair>())
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


var var::shift()
{
    if (heap())
        return heap()->shift();
    else
        return *this;
}


var& var::unshift(var iVar)
{
    if (!defined())
    {
        *this = iVar;
        return *this;
    }

    if (!heap())
        array();
    heap()->unshift(iVar);
    return *this;
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


ind var::index(var iVar) const
{
    int index;
    if (!defined())
        return -1;
    switch (atype())
    {
    case TYPE_PAIR:
        // Pairs are sorted
        index = binary(iVar);
        if ( (index < size()) && (heap()->key(index) == iVar) )
            return index;
        break;
    default:
        // Nothing else is sorted; start at the beginning
        for (ind i=0; i<size(); i++)
            if (at(i) == iVar)
                return i;
    }
    return -1;
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


/**
 * Usable both as a boolean (if this var uses a heap allocation) and as a means
 * to get the pointer.
 */
IHeap* var::heap() const
{
    if (reference())
        return mData.hp->derefHeap(~mType);
    return ( (mType == TYPE_ARRAY) && mData.hp ) ? mData.hp : 0;
}


/**
 * Reference constructor
 *
 * Dereferences iVar, constructing a reference to the resulting array.
 */
var var::reference(int iIndex) const
{
    if (type() != TYPE_ARRAY)
        throw error("var::reference: cannot reference non array");
    var v(*this);
    v.mType = -iIndex-1;
    return v;
}


/**
 * Converts a reference to an actual var in place
 *
 * However, if the ref is to another var, it takes a copy but returns a
 * reference to the original var.  That way, it can be modified by the caller
 * if necessary.
 */
var& var::dereference()
{
    if (!reference())
        return *this;

    var& r = varderef();
    if (&r != this)
    {
        // Set mType to not-a-reference, then let operator=() do the
        // housekeeping
        mType = 0;
        *this = r;
        return r;
    }
    else
    {
        // Normal de-reference
        assert(mData.hp);
        int index = ~mType;
        ind type = mData.hp->type();
        IHeap* tmp = mData.hp;
        mType = type;
        switch (type)
        {
        case TYPE_CHAR:
            mData.c = *mData.hp->ptrchar(index);
            break;
        case TYPE_INT:
            mData.i = *mData.hp->ptrint(index);
            break;
        case TYPE_LONG:
            mData.l = *mData.hp->ptrlong(index);
            break;
        case TYPE_FLOAT:
            mData.f = *mData.hp->ptrfloat(index);
            break;
        case TYPE_DOUBLE:
            mData.d = *mData.hp->ptrdouble(index);
            break;
        case TYPE_CFLOAT:
            mData.cf = *mData.hp->ptrcfloat(index) ;
            break;
        case TYPE_CDOUBLE:
            mType = TYPE_ARRAY;
            attach(new Heap(1, mData.hp->ptrcdouble(index)));
            break;
        default:
            throw error("var::dereference(): Unknown type");
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
    bool p =  // index on key rather than value
        (heap() && this->atype<pair>());
    while (lo != hi)
    {
        int pos = (hi-lo)/2 + lo;
        var x = p ? heap()->key(pos) : at(pos);
        if (x < iData)
            lo = pos+1;
        else
            hi = pos;
    }
    return hi;
}


/**
 * Assuming that *this is an array, returns a view of the array.  A view is
 * just another array, of type int, holding the dimensions of the new view.
 */
var var::view(const std::initializer_list<int> iList, int iOffset)
{
    if (!heap())
        throw error("var::view: Input not an array");
    var v;
    v.attach(new View(heap(), iList, iOffset));
    return v;
}


/**
 * Assuming that *this is an array, returns a view of the array.  A view is
 * just another array, of type int, holding the dimensions of the new view.
 */
var var::view(var iShape, int iOffset)
{
    if (!heap())
        throw error("var::view: Input not an array");
    var v;
    v.attach(new View(heap(), iShape, iOffset));
    return v;
}


/**
 * View initialiser function.  Allocates the underlying array as well as the
 * view.
 */
var libube::view(const std::initializer_list<int> iShape, var iType)
{
    int s = 1;
    for (const int* it=begin(iShape); it!=end(iShape); ++it)
        s *= *it;
    var v = iType ? iType : 0.0f;
    v.array();
    v.resize(s);
    return v.view(iShape);
}


/**
 * View initialiser function.  Allocates the underlying array as well as the
 * view.
 */
var libube::view(var iShape, var iType)
{
    int s = 1;
    for (int i=0; i<iShape.size(); i++)
        s *= iShape[i].cast<int>();
    var v = iType ? iType : 0.0f;
    v.array();
    v.resize(s);
    return v.view(iShape);
}


bool var::view() const
{
    return heap() && heap()->view();
};


/**
 * Yields a subview of dimension iDim, e.g., a row of a matrix, as a view given
 * offset iOffset into the original view.
 */
var var::subview(int iDim, ind iOffset)
{
    if (iDim < 1)
        throw error("var::subview(): subview must have dim > 0");
    var is = shape();
    for (int i=dim()-iDim; i>0; --i)
        is.shift();
    var iv = view(is, iOffset);
    return iv;
}


int var::dim() const
{
    if (!view())
        // It's not an array, or it's an array but not a view
        return 1;
    return heap()->dim();
}


int var::offset() const
{
    if (!view())
        return 0;
    return heap()->offset();
}


var& var::offset(int iOffset)
{
    if (!view())
        throw error("var::offset(): not a view");
    heap()->offset(iOffset);
    return *this;
}


/**
 * Returns a new array containing the shape of the view.
 */
var var::shape() const
{
    var s;
    for (int i=0; i<dim(); i++)
        s.push(shape(i));
    return s;
}


/**
 * Returns the size of the iDim'th dimension without allocating more memory for
 * the whole shape vector.
 */
int var::shape(int iDim) const
{
    if (iDim < 0)
        iDim = dim() + iDim;
    if (view())
        return heap()->shape(iDim);
    if (iDim > 0)
        throw error("var::shape(): dimension too large");
    return size();
}


int var::stride(int iDim) const
{
    if (view())
        return heap()->stride(iDim);
    if (iDim > 0)
        throw error("var::stride(): dimension too large");
    return 1;
}


void var::bounds(int iDim, int iIndex) const
{
    int d = shape(iDim);
    if (iIndex < 0 || iIndex >= d)
    {
        varstream s("var::bounds(): index = ");
        s << iIndex;
        s << ", but shape(" << iDim << ") is " << d;
        throw error(s);
    }
}


/**
 * error constructor
 *
 * This is really just a std::exception that takes a var as its argument.  The
 * var is formatted, so you can have arbitrary values appear in the what()
 * string.
 *
 * One thing wrong here is that what() should do the formatting, not the
 * constructor.  That is because memory is more likely to have been cleared by
 * the time what() gets executed.  It's this way because what() is const, and
 * str() and all the things returning strings aren't.  It also backtraces, so
 * there's perhaps some requirement for a non-backtracing exception that can be
 * cought.
 */
error::error(var iVar)
{
    varstream vs;
    vs << iVar;
    backTrace(vs);
    mVar = var(vs);
    mStr = mVar.str();
}

/**
 * Implements a Linux/MacOS backtrace.  In general, this should end up being
 * printed via what() by the terminate handler, and that puts a newline at the
 * end, so this doesn't.  Notice it allocates memory, so not suitable for an
 * out of memory exception.
 */
void error::backTrace(std::ostream& iStream)
{
#ifdef HAVE_BACKTRACE
    // Backtrace of more than a screenfull is probably not useful
    const int cBTSize = 64;
    void* callStack[cBTSize];

    // Get a backtrace and convert to (mangled) symbols
    int nCalls = backtrace(callStack, cBTSize);
    char** symbol = backtrace_symbols(callStack, nCalls);
    if (!symbol)
    {
        iStream << "\n  backtrace_symbols() failed";
        return;
    }
    int status;
    size_t length = 64;
    char* buffer = (char*)malloc(length);
    iStream << "\nCall stack (size " << nCalls << "):";
    for (int i=0; i<nCalls; i++)
    {
        // symbol[i] is of the form:
        //   file(function+offset)someotherstuff
        // We want to demangle the function name
        char* paren1 = strchr(symbol[i], '(');
        char* paren2 = strchr(symbol[i], ')');
        *paren1 = '\0';
        // Just the file, not very helpful
        //iStream << "\n  " << symbol[i] << ":\n    ";
        if (paren2 == paren1+1)
        {
            // It's an empty name
            iStream << "(undefined)";
            return;
        }
        char* plus = strchr(paren1+1, '+');
        *plus = '\0';
        buffer = abi::__cxa_demangle(paren1+1, buffer, &length, &status);
        iStream << "\n  ";  // Indent
        switch (status)
        {
        case 0:
            iStream << buffer;
            break;
        case -1:
            iStream << "Memory fuckup";
            break;
        case -2:
            iStream << paren1+1;
            break;
        case -3:
            iStream << "Invalid";
            break;
        }
    }
    free(buffer);
    free(symbol);
#endif // HAVE_BACKTRACE
}

const char* error::what() const noexcept
{
    return mStr;
}


// It's a bit verbose without these.  The duration could be "milliseconds", but
// that's an integer value whereas the double does carry meaningful extra
// precision.
using namespace std::chrono;
typedef duration<double,std::ratio<1,1000>> ms;

/**
 * Stores the current time at instantiation, along with an identifier.
 */
timer::timer(var iName)
{
    mName = iName;
    mBegan = steady_clock::now();
}

/**
 * Calculates and prints in milliseconds the difference between the current
 * time and that when the class was constructed.
 */
timer::~timer()
{
    steady_clock::time_point now = steady_clock::now();
    ms span = duration_cast<ms>(now - mBegan);
    std::cout << mName << ": " << span.count() << " ms" << std::endl;
}
