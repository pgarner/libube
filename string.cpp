/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#include <stdexcept>
#include <cassert>
#include <cstring>
#include <cstdarg>
#include <boost/regex.hpp>

#include "var.h"

/**
 * Read a line from an istream
 */
var& var::getline(std::istream& iStream)
{
    // Start off as a zero length string
    if (type() != TYPE_CHAR)
        *this = "";
    else
        resize(0);

    // sgetc()  return this character
    // sbumpc() return this character & advance
    // snextc() advance & return next character
    std::streambuf* buf = iStream.rdbuf();
    if (buf->sgetc() == EOF)
    {
        // The stream is finished; return undef
        return clear();
    }

    while (buf->sgetc() != EOF)
    {
        char c = buf->sbumpc();
        if (c == '\n') // Unix specific!
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
    const char* source = &s;
    const char* p;
    if (iMax != 1)
        while ( (p = std::strstr(source, iStr)) )
        {
            int len = p-source;
            if (len == 0)
            {
                source += strLen;
                continue;
            }

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
    if (type() == TYPE_CHAR)
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

var& var::strip()
{
    var& deref = dereference();
    if (type() != TYPE_CHAR)
        return *this;

    char* ptr = &deref;
    int leading = 0;
    while ((leading < deref.size()) && (isspace(ptr[leading])))
        leading++;
    int trailing = 0;
    while ((trailing < deref.size()) && (isspace(ptr[size()-trailing-1])))
        trailing++;

    int newSize = deref.size() - leading - trailing;
    if (leading > 0)
        std::memmove(ptr, ptr+leading, newSize);
    if (newSize != deref.size())
        deref.resize(newSize);
    return deref;
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
        // Bear in mind that libvar handles the null terminator
        // implicitly.
        va_start(ap, iFormat);
        int m = vsnprintf(&(*this), size(), iFormat, ap);
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

/**
 * Waiting for std::regex_search(), but currently implemented using
 * boost.
 */
bool var::search(var iRE)
{
    if (type() != TYPE_CHAR)
        throw std::runtime_error("var::search(): Not a string");
    boost::regex rgx(&iRE);
    bool r = boost::regex_search(&(*this), rgx);
    return r;
}

/**
 * Waiting for std::regex_match(), but currently implemented using
 * boost.
 */
bool var::match(var iRE)
{
    if (type() != TYPE_CHAR)
        throw std::runtime_error("var::match(): Not a string");
    boost::regex rgx(&iRE);
    bool r = boost::regex_match(&(*this), rgx);
    return r;
}

/**
 * Waiting for std::regex_replace(), but currently implemented using
 * boost.
 */
var var::replace(var iRE, var iStr)
{
    if (reference())
        return deref(*this).replace(iRE, iStr);
    if (type() != TYPE_CHAR)
        throw std::runtime_error("var::replace(): Not a string");

    std::string str = &(*this);
    boost::regex rgx(&iRE);
    var r = boost::regex_replace(str, rgx, &iStr).c_str();
    return r;
}
