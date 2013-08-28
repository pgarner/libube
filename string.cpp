/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2013
 */

#include <stdexcept>
#include <cstring>

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
    const char* source = s.cast<char*>();
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
            r.insert(0, at(0));
        else
        {
            r.insert(r.size(), s);
            r.insert(r.size(), at(i));
        }
    }

    return r;
}
