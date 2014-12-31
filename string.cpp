/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#include <stdexcept>
#include <locale>
#include <cassert>
#include <cstring>
#include <cstdarg>

// Waiting for std::regex_xxx(), but currently boost appears more reliable.
#include <boost/regex.hpp>

#include "var.h"


namespace libvar
{
    /*
     * The instantiations of the string functors defined in this module.  They
     * are declared extern in var.h
     */
    ToUpper toupper;
    ToLower tolower;
    Strip strip;
}


using namespace libvar;


var StringFunctor::operator ()(const var& iVar) const
{
    var v = alloc(iVar);
    if (iVar.atype<char>())
        string(iVar, v);
    else
        broadcast(iVar, v);
    return v;
}


var StringFunctor::operator ()(const var& iVar, var& oVar) const
{
    if (iVar.atype<char>())
        string(iVar, oVar);
    else
        broadcast(iVar, oVar);
    return oVar;
}


void StringFunctor::string(const var& iVar, var& oVar) const
{
    broadcast(iVar, oVar);
}


/**
 * String broadcaster
 *
 * Broadcasts the operation against iVar.
 */
void StringFunctor::broadcast(var iVar, var& oVar) const
{
    // This could be parallel!
    for (int i=0; i<iVar.size(); i++)
    {
        var ref = oVar.at(i);
        operator()(iVar.at(i), ref);
    }
}


RegExFunctor::RegExFunctor(var iRE)
{
    mRE = new boost::regex(iRE.str());
}

RegExFunctor::~RegExFunctor()
{
    // This (and other) reinterpret_cast<>s are to avoid var.h including boost
    // headers; this in turn would slow down compilation.
    delete reinterpret_cast<boost::regex*>(mRE);
}


/**
 * Read a line from an istream
 */
var& var::getline(std::istream& iStream)
{
    // Peek ahead to set eof if necessary, return nil if done
    iStream.peek();
    if (iStream.eof())
        return clear();

    // Start off as a zero length string as an array; re-use the existing array
    // if possible
    if (!defined() || !atype<char>())
        *this = "";
    else
        resize(0);
    array();

    // Read it
    while (!iStream.eof())
    {
        char c = iStream.get();
        if (c == '\n') // Unix specific?  ...no, I think c++ will change it to
                       // whatever is appropriate
            break;
        push(c);
    }
    return *this;
}

/**
 * A method modelled on ruby's split.
 *
 * ...which is of course modelled on perl's split.
 * returns an array of strings as a var.
 */
var var::split(const char* iStr, int iMax) const
{
    var r;
    int strLen = std::strlen(iStr);
    var s = *this;
    const char* source = s.ptr<char>();
    const char* p;
    if (iMax != 1)
        while ( (p = std::strstr(source, iStr)) )
        {
            int len = p-source;
            var s(len, source);
            r.push(s);
            source = p+strLen;

            if (iMax && (r.size() >= iMax-1))
                break;
        }
    r.push(source);

    return r;
}

var var::join(const char* iStr) const
{
    if (atype<char>())
        return *this;

    var r = "";
    var s = iStr;
    for (int i=0; i<size(); i++)
    {
        if (i == 0)
            r.insert(at(0));
        else
        {
            r.insert(s, r.size());
            r.insert(at(i), r.size());
        }
    }

    return r;
}


void Strip::string(const var& iVar, var& oVar) const
{
    char* ip = const_cast<var&>(iVar).ptr<char>();
    char* op = oVar.ptr<char>();
    int is = iVar.size();
    int leading = 0;
    while ((leading < is) && (isspace(ip[leading])))
        leading++;
    int trailing = 0;
    while ((trailing < is) && (isspace(ip[is-trailing-1])))
        trailing++;

    int newSize = is - leading - trailing;
    if (iVar.is(oVar))
    {
        if (leading > 0)
            std::memmove(op, op+leading, newSize);
        if (newSize != oVar.size())
            oVar.resize(newSize);
    }
    else
    {
        oVar = "";
        oVar.resize(newSize);
        for (int i=0; i<newSize; i++)
            op[i] = ip[i+leading];
    }
}

var& var::sprintf(const char* iFormat, ...)
{
    va_list ap;
    int n = 8; 
    clear();
    mType = TYPE_CHAR;
    resize(n);

    while (1)
    {
        // Bear in mind that libvar handles the null terminator implicitly.
        va_start(ap, iFormat);
        int m = vsnprintf(ptr<char>(), size(), iFormat, ap);
        va_end(ap);
        if (m < 0)
            throw std::runtime_error("var::sprintf(): Some error");
        if (m <= size())
            return *this;
        if (m > size())
            resize(m);
    }

    // Shouldn't get here
    assert(0);
    return *this;
}


void Search::string(const var& iVar, var& oVar) const
{
    boost::regex re = *reinterpret_cast<boost::regex*>(mRE);
    bool r = boost::regex_search(const_cast<var&>(iVar).str(), re);
    if (!r)
        oVar = nil;
    else
        if (!iVar.is(oVar))
            oVar = iVar;
}

var var::search(var iRE)
{
    Search s(iRE);
    return s(*this, *this);
}


void Match::string(const var& iVar, var& oVar) const
{
    boost::regex re = *reinterpret_cast<boost::regex*>(mRE);
    bool r = boost::regex_match(const_cast<var&>(iVar).str(), re);
    if (!r)
        oVar = nil;
    else
        if (!iVar.is(oVar))
            oVar = iVar;
}

var var::match(var iRE)
{
    Match m = Match(iRE);
    return m(*this, *this);
}


Replace::Replace(var iRE, var iStr)
    : RegExFunctor(iRE)
{
    mStr = iStr;
}

void Replace::string(const var& iVar, var& oVar) const
{
    boost::regex re = *reinterpret_cast<boost::regex*>(mRE);
    std::string s = const_cast<var&>(iVar).str();
    var str(mStr); // const thing
    var r = boost::regex_replace(s, re, str.str()).c_str();
    oVar = r;
}

var var::replace(var iRE, var iStr)
{
    Replace r = Replace(iRE, iStr);
    return r(*this, *this);
}


void ToUpper::string(const var& iVar, var& oVar) const
{
    if (!iVar.is(oVar))
    {
        oVar = "";
        oVar.resize(iVar.size());
    }
    char* ip = const_cast<var&>(iVar).ptr<char>();
    char* op = oVar.ptr<char>();
    for (int i=0; i<iVar.size(); i++)
        // Does not work for UTF-8 'é' and the like
        op[i] = std::toupper(ip[i]);
}

void ToLower::string(const var& iVar, var& oVar) const
{
    if (!iVar.is(oVar))
    {
        oVar = "";
        oVar.resize(iVar.size());
    }
    char* ip = const_cast<var&>(iVar).ptr<char>();
    char* op = oVar.ptr<char>();
    for (int i=0; i<iVar.size(); i++)
        // Does not work for UTF-8 'é' and the like
        op[i] = std::tolower(ip[i]);
}
