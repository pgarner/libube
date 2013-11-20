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
    VDEBUG(std::cout << typeOf() << "[" << size() << "]");
    VDEBUG(std::cout << std::endl);
    detach();
}


/**
 * Copy constructor.
 *
 * This one gets called when vars get passed as function parameters
 * and the like; also when vars are returned from functions, assuming
 * it's not optimised out.  For operator[] to work, this function
 * cannot dereference.
 */
var::var(const var& iVar)
{
    VDEBUG(std::cout << "Const copy" << std::endl);
    mData = iVar.mData;
    mIndex = iVar.mIndex;
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

    iVar.dereference();
    if (reference())
    {
        // We are a reference; have to write directly into a typed
        // array.  However, no new storage is allocated, so no need to
        // detach from old storage
        mData.hp->set(iVar, -mIndex-1);
    }
    else
    {
        // Not a reference.  Record whether the old value was a heap
        // allocation. If so, detach it after attaching the new value.
        varheap* tmp = heap() ? mData.hp : 0;
        mData = iVar.mData;
        mIndex = iVar.mIndex;
        mType = iVar.mType;
        attach();
        if (tmp)
            detach(tmp);
    }
    return *this;
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

var::var(int iSize, const char* iData)
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char*): " << iData << std::endl);
    mType = TYPE_ARRAY;
    mData.hp = new varheap(iSize, iData);
    mIndex = 0;
    attach();
}

var::var(const char* iData)
    : var(std::strlen(iData), iData)
{
    // It's all in the init
}

var::var(int iSize, char* const* iData)
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(char**): " << iData[0] << std::endl);
    mData.hp = 0;
    mType = TYPE_ARRAY;
    mIndex = 0;
    for (int i=0; i<iSize; i++)
        push(iData[i]);
}


var::var(int iSize, const int* iData)
{
    assert(iSize >= 0);
    VDEBUG(std::cout << "Ctor(int*): " << iData << std::endl);
    mType = TYPE_ARRAY;
    mData.hp = new varheap(iSize, iData);
    mIndex = 0;
    attach();
}


var::var(int iSize, int iFirst, ...)
{
    va_list ap;
    va_start(ap, iFirst);
    mData.i = iFirst;
    mIndex = 0;
    mType = TYPE_INT;
    for (int i=1; i<iSize; i++)
        push(va_arg(ap, int));
    va_end(ap);
}


var::var(int iSize, float iFirst, ...)
{
    va_list ap;
    va_start(ap, iFirst);
    mData.f = iFirst;
    mIndex = 0;
    mType = TYPE_FLOAT;
    for (int i=1; i<iSize; i++)
        push((float)(va_arg(ap, double)));
    va_end(ap);
}


/**
 * Equality operator
 *
 * It turns out that the negation is easier to code; i.e., there are
 * lots of not equal cases where you just want to bail out as soon as
 * you know it's not equal.
 */
bool var::operator ==(var iVar) const
{
    return !(*this != iVar);
}


bool var::operator !=(var iVar) const
{
    if (reference())
        return deref(*this).operator !=(iVar);
    iVar.dereference();
    if (mType != iVar.mType)
        return true;
    if (size() != iVar.size())
        return true;
    if (mType != TYPE_ARRAY)
        switch (mType)
        {
        case TYPE_CHAR:
            return (mData.c != iVar.mData.c);
        case TYPE_INT:
            return (mData.i != iVar.mData.i);
        case TYPE_LONG:
            return (mData.l != iVar.mData.l);
        case TYPE_FLOAT:
            return (mData.f != iVar.mData.f);
        case TYPE_DOUBLE:
            return (mData.d != iVar.mData.d);
        default:
            throw std::runtime_error("operator !=(): Unknown type");
        }
    for (int i=0; i<size(); i++)
        if (at(i) != iVar.at(i))
            return true;
    return false;
}


/*
 * Operator <
 *
 * This is the one that gets used by std::map in its search.
 */
bool var::operator <(var iVar) const
{
    if (reference())
        return deref(*this).operator <(iVar);
    iVar.dereference();

    if (mType != iVar.mType)
        return (mType < iVar.mType);
    if ( (mType != TYPE_ARRAY) && (iVar.mType != TYPE_ARRAY) )
        switch (mType)
        {
        case TYPE_CHAR:
            return (mData.c < iVar.mData.c);
        case TYPE_INT:
            return (mData.i < iVar.mData.i);
        case TYPE_LONG:
            return (mData.l < iVar.mData.l);
        case TYPE_FLOAT:
            return (mData.f < iVar.mData.f);
        case TYPE_DOUBLE:
            return (mData.d < iVar.mData.d);
        default:
            throw std::runtime_error("operator <(): Unknown type");
        }

    if ( heap() && (type() == TYPE_CHAR) &&
         iVar.heap() && (iVar.type() == TYPE_CHAR) )
        return (std::strcmp(mData.hp->ref(), iVar.mData.hp->ref()) < 0);
    for (int i=0; i<std::min(size(), iVar.size()); i++)
        if (at(i) < iVar.at(i))
            return true;
    return false;
}


/**
 * Shallow copy
 *
 * It's shallow in that a new array is created, but if that array
 * itself points to other arrays, they are not duplicated.
 */
var var::copy() const
{
    // Deref and return if there's no heap
    if (reference())
        return deref(*this).copy();
    if (!heap())
        return *this;

    // It's a heap
    var r;
    r.mData.hp = new varheap(*mData.hp);
    r.attach();
    return r;
}


bool var::defined() const
{
    // = not undefined
    return !( (mType == TYPE_ARRAY) && !mData.hp );
}


int var::size() const
{
    if (mType == TYPE_ARRAY)
        return mData.hp ? mData.hp->size() : 0;
    return 1;
}


var::dataEnum var::type() const
{
    if (mType == TYPE_ARRAY)
    {
        if (mData.hp)
            return mData.hp->type();
    }
    return mType;
}


/**
 * Cast to given type
 *
 * It may or may not be a cast, depending on the actual storage.
 * Return type is always the target of the cast, but it happens in
 * place because the cast may have to allocate memory.
 */
template<class T>
T var::cast()
{
    dereference();

    if (heap())
    {
        if (type() == TYPE_CHAR)
            *this = static_cast<T>(mData.hp->strtold());
        else
            throw std::runtime_error("cast(): Cannot cast array");
        // and drop though...
    }

    T r;
    switch (mType)
    {
    case TYPE_CHAR:
        *this = r = static_cast<T>(mData.c);
        return r;
    case TYPE_INT:
        r = static_cast<T>(mData.i);
        *this = r;
        return r;
    case TYPE_LONG:
        *this = r = static_cast<T>(mData.l);
        return r;
    case TYPE_FLOAT:
        *this = r = static_cast<T>(mData.f);
        return r;
    case TYPE_DOUBLE:
        *this = r = static_cast<T>(mData.d);
        return r;
    default:
        throw std::runtime_error("cast(): Unknown type");
    }

    // Should not get here
    return r;
}


/**
 * Cast to char* is a specialisation
 */
template<>
char* var::cast<char*>()
{
    dereference();

    if (heap())
    {
        if (mData.hp->type() == TYPE_CHAR)
            return &(*this);
    }

    vstream vs;
    vs << *this;
    *this = vs.var();

    return &(*this);
}


var& var::operator +=(var iVar)
{
    switch (mType)
    {
    case TYPE_ARRAY:
        if (type() == TYPE_CHAR)
            append(iVar);
        else
            mData.hp->add(iVar, (mIndex < 0) ? -mIndex-1 : -1);
        break;
    case TYPE_CHAR:
        mData.c += iVar.cast<char>();
        break;
    case TYPE_INT:
        mData.i += iVar.cast<int>();
        break;
    case TYPE_LONG:
        mData.l += iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f += iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d += iVar.cast<double>();
        break;
    default:
        throw std::runtime_error("operator +=(): Unknown type");
    }

    return *this;
}


var& var::operator -=(var iVar)
{
    switch (mType)
    {
    case TYPE_ARRAY:
        mData.hp->sub(iVar, (mIndex < 0) ? -mIndex-1 : -1);
        break;
    case TYPE_CHAR:
        mData.c -= iVar.cast<char>();
        break;
    case TYPE_INT:
        mData.i -= iVar.cast<int>();
        break;
    case TYPE_LONG:
        mData.l -= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f -= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d -= iVar.cast<double>();
        break;
    default:
        throw std::runtime_error("operator -=(): Unknown type");
    }

    return *this;
}


var& var::operator *=(var iVar)
{
    switch (mType)
    {
    case TYPE_ARRAY:
        mData.hp->mul(iVar, (mIndex < 0) ? -mIndex-1 : -1);
        break;
    case TYPE_CHAR:
        mData.c *= iVar.cast<char>();
        break;
    case TYPE_INT:
        mData.i *= iVar.cast<int>();
        break;
    case TYPE_LONG:
        mData.l *= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f *= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d *= iVar.cast<double>();
        break;
    default:
        throw std::runtime_error("operator *=(): Unknown type");
    }

    return *this;
}


var& var::operator /=(var iVar)
{
    switch (mType)
    {
    case TYPE_ARRAY:
        mData.hp->div(iVar, (mIndex < 0) ? -mIndex-1 : -1);
        break;
    case TYPE_CHAR:
        mData.c /= iVar.cast<char>();
        break;
    case TYPE_INT:
        mData.i /= iVar.cast<int>();
        break;
    case TYPE_LONG:
        mData.l /= iVar.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f /= iVar.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d /= iVar.cast<double>();
        break;
    default:
        throw std::runtime_error("operator /=(): Unknown type");
    }

    return *this;
}


var var::operator +(var iVar) const
{
    iVar.dereference();
    var r = copy();
    r += iVar;
    return r;
}

var var::operator -(var iVar) const
{
    iVar.dereference();
    var r = copy();
    r -= iVar;
    return r;
}

var var::operator *(var iVar) const
{
    iVar.dereference();
    var r = copy();
    r *= iVar;
    return r;
}

var var::operator /(var iVar) const
{
    iVar.dereference();
    var r = copy();
    r /= iVar;
    return r;
}


/**
 * Get a pointer to the actual storage
 *
 * Returns char* as that's what is often needed; must be cast
 * otherwise.
 */
char* var::operator &()
{
    if (reference())
        return deref(*this).operator &();
    if (heap())
        return mData.hp->ref();
    return (char*)&mData;
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
    if (reference())
        return deref(*this).operator [](iIndex);
    if (iIndex < 0)
        throw std::runtime_error("operator [int]: Negative index");
    if (iIndex >= size())
        resize(iIndex+1);
    var r(*this);
    r.mIndex = -iIndex-1;
    return r;
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
    if (reference())
        return deref(*this).operator [](iVar);
    if (!defined())
    {
        // A kind of constructor
        mType = TYPE_ARRAY;
        mData.hp = new varheap(0, TYPE_PAIR);
        mIndex = 0;
        attach();
    }
    else
        if (type() != TYPE_PAIR)
            throw std::runtime_error("operator [var]: Not a map");

    int index = binary(iVar);
    if ( (index >= size()) || (mData.hp->key(index) != iVar) )
        insert(iVar, index);
    var r(*this);
    r.mIndex = -index-1;
    return r;
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
    int dim = size() / 2;
    int p = iFirst * stride(0);
    bounds(0, iFirst);
    for (int i=1; i<dim; i++)
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
 * const.  That follows from it's being unable to change the array
 * size.
 */
var var::at(int iIndex) const
{
    if (reference())
        return deref(*this).at(iIndex);
    if (!defined())
        throw std::runtime_error("var::at(): uninitialised");
    if (mType == TYPE_ARRAY)
    {
        var r(*this);
        r.mIndex = -iIndex-1;
        return r;
    }
    if (iIndex == 0)
        return *this;
    throw std::runtime_error("var::at(): Index out of bounds");
}


std::ostream& operator <<(std::ostream& iStream, var iVar)
{
    iVar.dereference();
    switch (iVar.mType)
    {
    case var::TYPE_ARRAY:
        if (iVar.mData.hp)
            iVar.mData.hp->format(iStream);
        else
            iStream << "nil";
        break;
    case var::TYPE_CHAR:
        iStream << "\'";
        iStream << iVar.mData.c;
        iStream << "\'";
        break;
    case var::TYPE_INT:
        iStream << iVar.mData.i;
        break;
    case var::TYPE_LONG:
        iStream << iVar.mData.l;
        break;
    case var::TYPE_FLOAT:
        iStream << iVar.mData.f;
        break;
    case var::TYPE_DOUBLE:
        iStream << iVar.mData.d;
        break;
    default:
        throw std::runtime_error("<<(): Unknown type");
    }

    return iStream;
}


var& var::resize(int iSize)
{
    VDEBUG(std::cout << "var::resize(" << iSize << ")" << std::endl);
    assert(iSize >= 0);
    if (!heap())
    {
        // Not allocated
        if (heap(iSize))
        {
            // Need to allocate
            if (!defined())
            {
                mData.hp = new varheap(iSize, mType);
                attach();
            }
            else
            {
                var tmp = *this;
                mData.hp = new varheap(iSize, mType);
                mType = TYPE_ARRAY;
                attach();
                at(0) = tmp;
            }
        }
    }
    else
    {
        // It's allocated already
        mData.hp->resize(iSize);
    }
    return *this;
}


int var::attach()
{
    if (heap())
        return mData.hp->attach();
    return 0;
}


int var::detach(varheap* iHeap)
{
    if (iHeap)
        return iHeap->detach();
    if (heap())
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
        s += typeOf(type());
        s += "]";
        return s;
    }
    return typeOf(mType);
}


var& var::push(var iVar)
{
    VDEBUG(std::cout << "push: ");
    VDEBUG(std::cout << iVar.typeOf() << " " << typeOf());
    VDEBUG(std::cout << std::endl);
    if (!defined())
    {
        // Uninitialised
        mType = iVar.mType;
    }
    else
    {
        // Already initialised
        if ( (mType != TYPE_ARRAY) && (iVar.mType != mType) )
            throw std::runtime_error("push(): Incompatible types");
    }
        
    int last = size();
    resize(last+1);
    at(last) = iVar;
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
    if (reference())
        return deref(*this).insert(iVar, iIndex);
    if (iIndex > size())
        throw std::range_error("insert(): index too large");

    iVar.dereference();
    if (type() == TYPE_VAR)
    {
        // Implies array; insert a single var
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
            at(i) = at(i-1);
        at(iIndex) = iVar;
    }
    else if (type() == TYPE_PAIR)
    {
        // Implies array; insert a single pair
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
        {
            mData.hp->key(i) = mData.hp->key(i-1);
            at(i) = at(i-1);
        }
        mData.hp->key(iIndex) = iVar;
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
    if (reference())
        return deref(*this).remove(iIndex);
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


/**
 * Sets the value to the equivalent of undefined
 */
var& var::clear()
{
    detach();
    mData.hp = 0;
    mIndex = 0;
    mType = TYPE_ARRAY;
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
    if (reference())
        return deref(*this).sum();
    var sum = 0.0;
    for (int i=0; i<size(); i++)
        sum += at(i);
    return sum;
}


/* Quite likely to be unstable in general */
var var::prod() const
{
    var d(*this);
    d.dereference();
    var prod = 1.0;
    for (int i=0; i<d.size(); i++)
        prod *= d.at(i);
    return prod;
}


/**
 * True if this var uses a heap allocation.
 */
bool var::heap(int iSize) const
{
    // iSize defaults to -1
    if (iSize < 0)
        return ( (mType == TYPE_ARRAY) && mData.hp );
    if ( ((mType != TYPE_ARRAY) && (iSize > 1)) ||
         ((mType == TYPE_ARRAY) && (iSize >= 0)) )
        return true;
    return false;
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
 * deref() function
 *
 * Trivially calls the dereference() method, but has the effect of
 * copying the var so the caller can be const.
 *
 * Can't overload the name dereference(); why?
 */
var& deref(var iVar)
{
    return iVar.dereference();
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

    int index = -mIndex-1;
    if (type() == TYPE_VAR)
    {
        // Set mIndex to not-a-reference, then let operator=() do the
        // housekeeping
        mIndex = 0;
        var& r = mData.hp->ref<var>(index);
        *this = r;
        return r;
    }
    else if (type() == TYPE_PAIR)
    {
        // Set mIndex to not-a-reference, then let operator=() do the
        // housekeeping
        mIndex = 0;
        var& r = mData.hp->ref<pair>(index).val;
        *this = r;
        return r;
    }
    else
    {
        // Normal de-reference
        assert(heap());
        varheap* tmp = mData.hp;
        mType = type();
        mIndex = 0;
        switch (mType)
        {
        case TYPE_CHAR:
            mData.c = mData.hp->ref<char>(index);
            break;
        case TYPE_INT:
            mData.i = mData.hp->ref<int>(index);
            break;
        case TYPE_LONG:
            mData.l = mData.hp->ref<long>(index);
            break;
        case TYPE_FLOAT:
            mData.f = mData.hp->ref<float>(index);
            break;
        case TYPE_DOUBLE:
            mData.d = mData.hp->ref<double>(index);
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
    bool pair = (type() == TYPE_PAIR); // index on key rather than value
    while (lo != hi)
    {
        int pos = (hi-lo)/2 + lo;
        var x = pair ? mData.hp->key(pos) : at(pos);
        if (x < iData)
            lo = pos+1;
        else
            hi = pos;
    }
    return hi;
}


/**
 * Assuming that *this is an array, returns a view of the array.  A
 * view is just another array of type int holding the dimensions of
 * the new view.
 */
var var::view(int iDim, int iFirst, ...)
{
    va_list ap;
    va_start(ap, iFirst);

    // The dimension vector must be an array, even if only 1D
    var v(2, iFirst, 0);

    // First entry of each pair is the dimension
    for (int i=1; i<iDim; i++)
    {
        v.push(va_arg(ap, int));
        v.push(0);
    }
    va_end(ap);

    // The second of each pair is the stride
    int p = 1;
    for (int i=iDim-1; i>=0; i--)
    {
        v[i*2+1] = p;
        p *= v.mData.hp->viewRef(i*2);
    }

    // the p that drops out should be the overall size
    if (!defined())
        resize(p);
    else
        if (p != size())
            throw std::runtime_error("var::view(): Incompatible dimensions");

    // Tell the heap object that it's a view
    assert(mType == TYPE_ARRAY);
    v.mData.hp->setView(mData.hp);

    return v;
}


int var::shape(int iDim) const
{
    if (!mData.hp || !mData.hp->view())
        throw std::runtime_error("var::shape(): Not a view");
    return mData.hp->viewRef(iDim*2);
}


int var::stride(int iDim) const
{
    if (!mData.hp || !mData.hp->view())
        throw std::runtime_error("var::stride(): Not a view");
    return mData.hp->viewRef(iDim*2+1);
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
    setp(&mVar, &mVar);
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
