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
#include <cassert>

#include <lube/var.h>

namespace libube
{
    /**
     * Ad-hoc JSON (JavaScript Object Notation) parser and writer
     * (see http://json.org/)
     */
    class JSON
    {
    public:
        var operator ()(std::istream& iStream);
        void format(std::ostream& iStream, var iVar, int iIndent = 0);
    private:
        void formatArray(std::ostream& iStream, var iVar, int iIndent);
        void formatView(std::ostream& iStream, var iVar, int iIndent);
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
    varstream vs(iStr);
    long l;
    vs >> l;
    if (!vs.fail())
        return l;

    // Try for double
    vs.clear();
    vs.seekg(0);
    double d;
    vs >> d;
    if (!vs.fail())
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


/**
 * Never indent a basic var, but do pass the current level along to the array
 * formatter.
 */
void JSON::format(std::ostream& iStream, var iVar, int iIndent)
{
    switch (iVar.type())
    {
    case TYPE_ARRAY:
        if (iVar.heap())
            iVar.view()
                ? formatView(iStream, iVar, iIndent)
                : formatArray(iStream, iVar, iIndent);
        else
            iStream << "null";
        break;
    case TYPE_CHAR:
        // This is probably not JSON
        iStream << "\'";
        iStream << iVar.get<char>();
        iStream << "\'";
        break;
    case TYPE_INT:
        iStream << iVar.get<int>();
        break;
    case TYPE_LONG:
        iStream << iVar.get<long>();
        break;
    case TYPE_FLOAT:
        iStream << iVar.get<float>();
        break;
    case TYPE_DOUBLE:
        iStream << iVar.get<double>();
        break;
    case TYPE_CFLOAT:
        iStream << iVar.get<cfloat>();
        break;
    case TYPE_CDOUBLE:
        //iStream << iVar.get<cdouble>();
        throw error("var::format(): cdouble should be array");
        break;
    default:
        throw error("var::format(): Unknown type");
    }
}


// i: input variable; o: output stream...
std::ostream& libube::operator <<(std::ostream& iStream, var iVar)
{
    JSON json;
    json.format(iStream, iVar);
    return iStream;
}

void JSON::formatArray(std::ostream& iStream, var iVar, int iIndent)
{
    switch (iVar.atype())
    {
    case TYPE_CHAR:
        iStream << "\"";
        iStream << iVar.ptr<char>();
        iStream << "\"";
        break;
    case TYPE_PAIR:
        // Might be empty
        iStream << "{";
        for (int i=0; i<iVar.size(); i++)
        {
            iStream << "\n";
            for (int j=0; j<iIndent+2; j++)
                iStream << " ";
            iStream << iVar.key(i) << ": ";
            format(iStream, iVar[i], iIndent+2);
            if (i < iVar.size()-1)
                iStream << ",";
        }
        if (iVar.size())
        {
            iStream << "\n";
            for (int j=0; j<iIndent; j++)
                iStream << " ";
        }
        iStream << "}";
        break;
    case TYPE_VAR:
        iStream << "[\n";
        for (int i=0; i<iVar.size(); i++)
        {
            for (int j=0; j<iIndent+2; j++)
                iStream << " ";
            format(iStream, iVar[i], iIndent+2);
            if (i < iVar.size()-1)
                iStream << ",";
            iStream << "\n";
        }
        for (int j=0; j<iIndent; j++)
            iStream << " ";
        iStream << "]";
        break;
    case TYPE_CDOUBLE:
        // Don't call at(); it will just create more arrays & loop
        if (iVar.size() == 1)
            iStream << *iVar.ptr<cdouble>();
        else
        {
            iStream << "[\n";
            for (int i=0; i<iVar.size(); i++)
            {
                for (int j=0; j<iIndent+2; j++)
                    iStream << " ";
                iStream << iVar.ptr<cdouble>(i);
                if (i < iVar.size()-1)
                    iStream << ",";
                iStream << "\n";
            }
            for (int j=0; j<iIndent; j++)
                iStream << " ";
            iStream << "]";
        }
        break;
    default:
        iStream << "[";
        for (int i=0; i<iVar.size(); i++)
        {
            if (i != 0)
                iStream << ", ";
            iStream << iVar.at(i);
        }
        iStream << "]";
    }
}

// May still be too verbose
void JSON::formatView(std::ostream& iStream, var iVar, int iIndent)
{
    assert(iVar.heap());

    // Output shape if it's more than a matrix
    int nDim = iVar.dim();
    if (nDim > 2)
    {
        for (int i=0; i<nDim; i++)
        {
            iStream << iVar.shape(i);
            if (i != nDim-1)
                iStream << "x";
        }
        iStream << " tensor:" << std::endl;
    }

    // If it's less than 2D, just print it as with format
    if (nDim < 2)
    {
        iStream << "[";
        for (int i=0; i<iVar.size(); i++)
        {
            iStream << iVar[i];
            if (i != iVar.size()-1 )
                iStream << ", ";
        }
        iStream << "]";
        return;
    }

    // Calculate how many matrices we have
    int nMats = 1;
    for (int i=0; i<nDim-2; i++)
        nMats *= iVar.shape(i);

    // Format as a sequence of matrices
    int nRows = iVar.shape(nDim-2);
    int nCols = iVar.shape(nDim-1);
    for (int k=0; k<nMats; k++)
    {
        iStream << "[\n";
        for (int j=0; j<nRows; j++)
        {
            for (int k=0; k<iIndent+2; k++)
                iStream << " ";
            for (int i=0; i<nCols; i++)
            {
                iStream << iVar[k*nRows*nCols + j*nCols + i];
                if ( (j != nRows-1) || (i != nCols-1) )
                    iStream << ",";
                if (i != nCols-1)
                    iStream << " ";
            }
            iStream << "\n";
        }
        for (int j=0; j<iIndent; j++)
            iStream << " ";
        iStream << "]";
        if (k != nMats-1)
            iStream << std::endl;
    }
}
