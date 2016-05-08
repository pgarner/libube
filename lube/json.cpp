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
        char get(std::istream& iStream);
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
char JSON::get(std::istream& iStream)
{
    char got;
    do {
        iStream.get(got);
    } while (isspace(got));
    return got;
}

var JSON::doValue(std::istream& iStream)
{
    var val;
    switch (get(iStream))
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
        iStream.unget();
        val = doRaw(iStream);
    }
    return val;
}

var JSON::doObject(std::istream& iStream)
{
    var obj;
    obj[nil];
    var key;
    while (iStream)
    {
        switch (get(iStream))
        {
        case '}':
            return obj;
            break;
        case '"':
            key = doString(iStream);
            break;
        case ':':
            obj[key] = doValue(iStream);
            break;
        case ',':
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
    while (iStream)
    {
        switch (get(iStream))
        {
        case ']':
            return arr;
            break;
        case ',':
            break;
        default:
            iStream.unget();
            arr.push(doValue(iStream));
        }
    }
    return arr;
}

var JSON::doString(std::istream& iStream)
{
    var str;
    char got;
    // Not robust to escapes yet
    while (iStream.get(got))
        if (got == '"')
            break;
        else
            str.push(got);
    return str;
}

var JSON::doRaw(std::istream& iStream)
{
    // Read up to a space or JSON delimiter
    var val;
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
    
    // It's probably a number, but return a string for the moment
    return val;
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
