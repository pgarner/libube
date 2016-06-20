/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, March 2016
 */

#include <istream>
#include <cctype>
#include <cstring>
#include <lube/var.h>

namespace libube
{
    /**
     * Ad-hoc JSON (JavaScript Object Notation) parser; (see http://json.org/)
     */
    class JSON
    {
    public:
        var operator ()(std::istream& iStream);
    private:
        char peek(std::istream& iStream);
        var doValue(std::istream& iStream);
        var doObject(std::istream& iStream);
        var doArray(std::istream& iStream);
        var doString(std::istream& iStream);
        var doRaw(std::istream& iStream);
    };
}


using namespace libube;
using namespace std;

/** Find the next character after whitespace */
char JSON::peek(std::istream& iStream)
{
    char c = iStream.peek();
    while (isspace(c))
    {
        iStream.get();
        c = iStream.peek();
    }
    return c;
}

var JSON::doValue(std::istream& iStream)
{
    var val;
    switch (peek(iStream))
    {
    case '{':
        val = doObject(iStream);
        break;
    case '[':
        val = doArray(iStream);
        break;
    case '"':
        val = doString(iStream);
        break;
    default:
        val = doRaw(iStream);
    }
    return val;
}

var JSON::doObject(std::istream& iStream)
{
    var obj;
    obj[nil];
    var key;
    char got = iStream.get();
    if (got != '{')
        throw error("JSON object doesn't start with {");
    while (iStream)
    {
        switch (peek(iStream))
        {
        case '}':
            iStream.get();
            return obj;
            break;
        case '"':
            key = doString(iStream);
            break;
        case ':':
            iStream.get();
            obj[key] = doValue(iStream);
            break;
        case ',':
            iStream.get();
            key = nil;
            break;
        default:
            throw error("Unknown character");
        }
    }
    return obj;
}

var JSON::doArray(std::istream& iStream)
{
    var arr;
    char got = iStream.get();
    if (got != '[')
        throw error("JSON array doesn't start with [");
    while (iStream)
    {
        switch (peek(iStream))
        {
        case ']':
            iStream.get();
            return arr;
            break;
        case ',':
            iStream.get();
            break;
        default:
            arr.push(doValue(iStream));
        }
    }
    return arr;
}

var JSON::doString(std::istream& iStream)
{
    var str = "";
    char got = iStream.get();
    if (got != '\"')
        throw error("JSON string doesn't start with \"");
    while (iStream.get(got))
        switch (got)
        {
        case '\"':
            return str;
        case '\\':
            // Escape character
            got = iStream.get();
            switch (got)
            {
            case '\"':
                str.push(got);
                break;
            default:
                throw error("Unrecognised JSON string escape");
            }
            break;
        default:
            str.push(got);
        }
    throw error("Unterminated JSON string");
    return nil;
}

var toNumber(var iStr)
{
    // Try for integer
    varstream ls(iStr.copy());
    long l;
    ls >> l;
    if (!ls.fail())
        return l;

    // Try for double
    varstream ds(iStr.copy());
    double d;
    ds >> d;
    if (!ds.fail())
        return d;

    // Failed
    return nil;
}

var JSON::doRaw(std::istream& iStream)
{
    // Read up to a space or JSON delimiter
    var val = "";
    char c;
    while (iStream.get(c))
    {
        if (isspace(c))
            break;
        if (strchr(",[]{}", c))
            break;
        val.push(c);
    }

    // These values have special meanings
    if (val == "true")
        return 1;
    else if (val == "false")
        return 0;
    else if (val == "null")
        return nil;
    
    // It should be a number
    var num = toNumber(val);
    if (num)
        return num;

    // Broke
    var e = "Unrecognised JSON raw value: ";
    e.append(val);
    throw error(e);
}

/**
 * For proper JSON we should call doObject(), mandating it be in braces.  In
 * calling doValue() we're allowing just raw values which is OK for lube but
 * not strictly JSON.
 */
var JSON::operator()(std::istream& iStream)
{
    var val = doValue(iStream);
    return val;
}

std::istream& libube::operator >>(std::istream& iStream, var& ioVar)
{
    JSON json;
    var in = json(iStream);
    ioVar = in;
    return iStream;
}
