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
 * Template specialisations (before any get used)
 *
 * These can be used to get the actual storage in order to convert
 * back from var to the builtin type. Functions as var to type
 * conversion.  It's the opposite of the initialisation constructors;
 * C++ doesn't allow overloading on return type, so the return type
 * must be specified.
 */
template<> var& var::ref<var>(int iIndex) {
    assert(mData.hp);
    return mData.hp->mData.vp[iIndex];
}
template<> char& var::ref<char>(int iIndex) {
    return !heap() ? mData.c : mData.hp->mData.cp[iIndex];
}
template<> int& var::ref<int>(int iIndex) {
    return !heap() ? mData.i : mData.hp->mData.ip[iIndex];
}
template<> long& var::ref<long>(int iIndex) {
    return !heap() ? mData.l : mData.hp->mData.lp[iIndex];
}
template<> float& var::ref<float>(int iIndex) {
    return !heap() ? mData.f : mData.hp->mData.fp[iIndex];
}
template<> double& var::ref<double>(int iIndex) {
    return !heap() ? mData.d : mData.hp->mData.dp[iIndex];
}


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
 * This one gets called when vars gets passed as function parameters
 * and the like.
 */
var::var(const var& iVar)
{
    VDEBUG(std::cout << "Const copy" << std::endl);
    mData = iVar.mData;
    mIndex = iVar.mIndex;
    mType = iVar.mType;
    attach();
    dereference(); // which may detach() again
}

#if 0
// I think this one is unnecessary
var::var(var& iVar)
{
    VDEBUG(std::cout << "Copy" << std::endl);
    mData = iVar.mData;
    mIndex = iVar.mIndex;
    mType = iVar.mType;
    attach();
}
#endif


/**
 * Copy assignment
 *
 * This is the one that gets called when functions return vars.
 */
var& var::operator =(const var& iVar)
{
    VDEBUG(std::cout << "Copy assignment" << std::endl);

    if (reference())
    {
        // We are a reference; have to write directly into a typed
        // array.  However, no new storage is allocated, so no need to
        // detach from old storage
        var v(iVar);
        int index = -mIndex-1;
        switch (type())
        {
        case TYPE_VAR:
            ref<var>(index) = v;
            break;
        case TYPE_CHAR:
            ref<char>(index) = v.cast<char>();
            break;
        case TYPE_INT:
            ref<int>(index) = v.cast<int>();
            break;
        case TYPE_LONG:
            ref<long>(index) = v.cast<long>();
            break;
        case TYPE_FLOAT:
            ref<float>(index) = v.cast<float>();
            break;
        case TYPE_DOUBLE:
            ref<double>(index) = v.cast<double>();
            break;
        default:
            throw std::runtime_error("var::set(): Unknown type");
        }
        assert(mData.hp->mData.vp); // Any of the pointers
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

#if 0
// I think this one is unnecessary
var& var::operator =(var& iVar)
{
    VDEBUG(std::cout << "Basic copy assignment: " << iVar << std::endl);
    mData = iVar.mData;
    mIndex = iVar.mIndex;
    mType = iVar.mType;
    attach();
    return *this;
}
#endif


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


bool var::operator ==(const var& iVar) const
{
    return !(*this != iVar);
}


bool var::operator !=(const var& iVar) const
{
    var v1 = *this;
    v1.dereference();
    var v2 = iVar;
    if (v1.mType != v2.mType)
        return true;
    if (v1.size() != v2.size())
        return true;
    if (v1.mType != TYPE_ARRAY)
        switch (v1.mType)
        {
        case TYPE_CHAR:
            return (v1.mData.c != v2.mData.c);
        case TYPE_INT:
            return (v1.mData.i != v2.mData.i);
        case TYPE_LONG:
            return (v1.mData.l != v2.mData.l);
        case TYPE_FLOAT:
            return (v1.mData.f != v2.mData.f);
        case TYPE_DOUBLE:
            return (v1.mData.d != v2.mData.d);
        default:
            throw std::runtime_error("operator !=(): Unknown type");
        }
    for (int i=0; i<v1.size(); i++)
        if (v1.at(i) != v2.at(i))
            return true;
    return false;
}


bool var::operator <(const var& iVar) const
{
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
        return (std::strcmp(mData.hp->mData.cp, iVar.mData.hp->mData.cp) < 0);
    for (int i=0; i<std::min(size(), iVar.size()); i++)
        if (at(i) < iVar.at(i))
            return true;
    return false;
}


/**
 * Get a pointer to the actual storage
 *
 * Returns char* as that's what is often needed; must be cast
 * otherwise.
 */
char* var::operator &()
{
    if (heap())
        return mData.hp->mData.cp;
    return (char*)&mData;
}


/**
 * Shallow copy
 *
 * It's shallow in that a new array is created, but if that array
 * itself points to other arrays, they are not duplicated.
 */
var var::copy() const
{
    if (!heap())
        return *this;

    var r;
    r.mType = mType;
    r.resize(size());
    for (int i=0; i<size(); i++)
        r.set(i, at(i));
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
            return mData.hp->mType;
    }
    return mType;
}


/**
 * Cast to given type
 *
 * It may or may not be a cast, depending on the actual storage.
 * Return type is always var because the cast may have to allocate
 * memory.
 */
template<class T>
T var::cast()
{
    if (heap())
    {
        if (mType == TYPE_CHAR)
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
    if (heap())
    {
        if (mData.hp->mType == TYPE_CHAR)
            return mData.hp->mData.cp;
        else
            throw std::runtime_error("cast<char*>(): Cannot cast array (yet)");
    }

    const size_t s = 128;
    char tmp[s];
    int n;
    switch (mType)
    {
    case TYPE_CHAR:
        n = snprintf(tmp, s, "%c", mData.c);
        break;
    case TYPE_INT:
        n = snprintf(tmp, s, "%d", mData.i);
        break;
    case TYPE_LONG:
        n = snprintf(tmp, s, "%ld", mData.l);
        break;
    case TYPE_FLOAT:
        n = snprintf(tmp, s, "%f", mData.f);
        break;
    case TYPE_DOUBLE:
        n = snprintf(tmp, s, "%f", mData.d);
        break;
    default:
        throw std::runtime_error("cast<char*>(): Unknown type");
    }

    *this = var(n, tmp);
    return mData.hp->mData.cp;
}


// Could pass by value and save the tmp
var& var::operator +=(const var& iVar)
{
    var tmp = iVar;
    switch (mType)
    {
    case TYPE_ARRAY:
        if (mIndex < 0)
        {
            // It's a reference
            int index = -mIndex-1;
            // Like set()
            switch (type())
            {
            case TYPE_VAR:
                ref<var>(index) += tmp;
            case TYPE_CHAR:
                ref<char>(index) += tmp.cast<char>();
                break;
            case TYPE_INT:
                ref<int>(index) += tmp.cast<int>();
                break;
            case TYPE_LONG:
                ref<long>(index) += tmp.cast<long>();
                break;
            case TYPE_FLOAT:
                ref<float>(index) += tmp.cast<float>();
                break;
            case TYPE_DOUBLE:
                ref<double>(index) += tmp.cast<double>();
                break;
            default:
                throw std::runtime_error("var::+=(): Unknown type");
            }
        }
        break;
    case TYPE_CHAR:
        mData.c += tmp.cast<char>();
        break;
    case TYPE_INT:
        mData.i += tmp.cast<int>();
        break;
    case TYPE_LONG:
        mData.l += tmp.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f += tmp.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d += tmp.cast<double>();
        break;
    default:
        throw std::runtime_error("operator +=(): Unknown type");
    }

    return *this;
}


var& var::operator -=(const var& iVar)
{
    var tmp = iVar;
    switch (mType)
    {
    case TYPE_CHAR:
        mData.c -= tmp.cast<char>();
        break;
    case TYPE_INT:
        mData.i -= tmp.cast<int>();
        break;
    case TYPE_LONG:
        mData.l -= tmp.cast<long>();
        break;
    case TYPE_FLOAT:
        mData.f -= tmp.cast<float>();
        break;
    case TYPE_DOUBLE:
        mData.d -= tmp.cast<double>();
        break;
    default:
        throw std::runtime_error("operator -=(): Unknown type");
    }

    return *this;
}


/**
 * The operator[] just creates a reference.
 */
var var::operator [](int iIndex)
{
    if (!defined())
        throw std::runtime_error("operator []: Not defined");
    if (iIndex < 0)
        throw std::runtime_error("operator []: Negative index");
    if (mType != TYPE_ARRAY)
        throw std::runtime_error("operator []: Not an array");
    if (iIndex >= size())
        resize(iIndex+1);
    var r = *this;
    r.dereference();
    r.mIndex = -iIndex-1;
    return r;
}


std::ostream& operator <<(std::ostream& iStream, const var& iVar)
{
    if (iVar.size() == 0)
        return iStream;
    var v = iVar;
    v.dereference();
    if (v.heap())
    {
        // Array
        assert(iVar.mData.hp->mData.vp); // Any of the pointers
        assert(v.mData.hp->mData.vp); // Any of the pointers
        if (v.type() != var::TYPE_CHAR)
            iStream << "{";
        switch (v.type())
        {
        case var::TYPE_VAR:
            for (int i=0; i<v.size(); i++)
            {
                if (i != 0)
                    iStream << ", ";
                iStream << v.mData.hp->mData.vp[i];
            }
            break;
        case var::TYPE_CHAR:
            iStream << v.mData.hp->mData.cp;
            break;
        case var::TYPE_INT:
            for (int i=0; i<v.size(); i++)
            {
                if (i != 0)
                    iStream << ", ";
                iStream << v.mData.hp->mData.ip[i];
                if (i > 5)
                {
                    iStream << ", ...";
                    break;
                }
            }
            break;
        case var::TYPE_LONG:
        case var::TYPE_FLOAT:
        case var::TYPE_DOUBLE:
        default:
            throw std::runtime_error("<<(): Unknown type");
        }
        if (v.type() != var::TYPE_CHAR)
            iStream << "}";
    }
    else
        switch (v.mType)
        {
        case var::TYPE_CHAR:
            iStream << v.mData.c;
            break;
        case var::TYPE_INT:
            iStream << v.mData.i;
            break;
        case var::TYPE_LONG:
            iStream << v.mData.l;
            break;
        case var::TYPE_FLOAT:
            iStream << v.mData.f;
            break;
        case var::TYPE_DOUBLE:
            iStream << v.mData.d;
            break;
        default:
            throw std::runtime_error("<<(): Unknown type");
        }

    return iStream;
}


void var::resize(int iSize)
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
                set(0, tmp);
            }
        }
    }
    else
    {
        // It's allocated already
        mData.hp->resize(iSize);
    }
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


const char* var::typeOf()
{
    switch (mType)
    {
    case TYPE_ARRAY: return "array";
    case TYPE_CHAR: return "char";
    case TYPE_INT: return "int";
    case TYPE_LONG: return "long";
    case TYPE_FLOAT: return "float";
    case TYPE_DOUBLE: return "double";
    case TYPE_VAR: return "var";
    default:
        throw std::runtime_error("typeOf(): Unknown type");
    }
}


/**
 * Set the value at iIndex to the value of iVar
 */
void var::set(int iIndex, var iVar)
{
    if (iIndex >= size())
        resize(iIndex+1);

    var tmp = iVar;
    switch (type())
    {
    case TYPE_VAR:
        ref<var>(iIndex) = iVar;
        break;
    case TYPE_CHAR:
        ref<char>(iIndex) = tmp.cast<char>();
        break;
    case TYPE_INT:
        ref<int>(iIndex) = tmp.cast<int>();
        break;
    case TYPE_LONG:
        ref<long>(iIndex) = tmp.cast<long>();
        break;
    case TYPE_FLOAT:
        ref<float>(iIndex) = tmp.cast<float>();
        break;
    case TYPE_DOUBLE:
        ref<double>(iIndex) = tmp.cast<double>();
        break;
    default:
        throw std::runtime_error("var::set(): Unknown type");
    }    
}


var var::at(int iIndex) const
{
    if (!defined())
        throw std::runtime_error("var::at(): uninitialised");
    if (mType == TYPE_ARRAY)
        return mData.hp->at(iIndex);
    if (iIndex == 0)
        return *this;
    throw std::runtime_error("var::at(): Index out of bounds");
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
    set(last, iVar);
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
var& var::insert(int iIndex, var iVar)
{
    if (iIndex > size())
        throw std::range_error("insert(): index too large");
    if (type() == TYPE_VAR)
    {
        // Implies array; insert a single var
        resize(size()+1);
        for (int i=size()-1; i>iIndex; i--)
        {
            set(i, at(i-1));
        }
        set(iIndex, iVar);
    }
    else
    {
        // It's a fundamental type, insert the whole array
        resize(size()+iVar.size());
        for (int i=size()-1; i>iIndex+iVar.size()-1; i--)
            set(i, at(i-iVar.size()));
        for (int i=0; i<iVar.size(); i++)
            set(iIndex+i, iVar.at(i));
    }

    return *this;
}


var var::remove(int iIndex)
{
    assert(iIndex >= 0);
    if (iIndex > size()-1)
        throw std::range_error("remove(): index too large");
    var r = at(iIndex);
    for (int i=iIndex+1; i<size(); i++)
        set(i-1, at(i));
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
        r.insert(p, at(i));
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
 * Converts a reference to an actual var in place
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
        *this = mData.hp->mData.vp[index];
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
            mData.c = mData.hp->mData.cp[index];
            break;
        case TYPE_INT:
            mData.i = mData.hp->mData.ip[index];
            break;
        case TYPE_LONG:
            mData.l = mData.hp->mData.lp[index];
            break;
        case TYPE_FLOAT:
            mData.f = mData.hp->mData.fp[index];
            break;
        case TYPE_DOUBLE:
            mData.d = mData.hp->mData.dp[index];
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
    while (lo != hi)
    {
        int pos = (hi-lo)/2 + lo;
        if (iData < at(pos))
            hi = pos;
        else
            lo = pos+1;
    }
    return hi;
}
