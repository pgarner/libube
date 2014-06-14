/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2013
 */

#include <fstream>
#include <stdexcept>
#include <var.h>

#include "expat.h"


namespace libvar
{
    /**
     * Class to wrap the expat library and callbacks.
     */
    class Expat : public varfile
    {
    public:
        Expat();
        ~Expat();
        var parse(const char* iFile);
        void startElementHandler(const XML_Char *iName, const XML_Char **iAtts);
        void endElementHandler(const XML_Char *iName);
        void characterDataHandler(const XML_Char *iStr, int iLen);
        virtual var read(const char* iFile);
        virtual void write(const char* iFile, var iVar);

    private:
        var mVar;            ///< Var to populate during parse
        var mStack;          ///< Parse stack
        XML_Parser mParser;  ///< The expat parser

        var element();
    };
}


using namespace libvar;


void libvar::factory(varfile** oFile)
{
    *oFile = new Expat;
}


/**
 * In principle this is unnecessary; in practice it's useful to catch
 * errors (of the form "dude, you forgot to implement...").
 */
static void DefaultHandler(
    void *iUserData, const XML_Char *iStr, int iLen
)
{
    var str(iLen, iStr);
    std::cout << "expat::DefaultHandler: " << str << std::endl;
    throw std::runtime_error("xmlfile: default handler called");
}


/**
 * We don't do anything with the XML declaration, but need to handle
 * it so the the default handler doesn't.
 */
static void XmlDeclHandler(
    void *iUserData,
    const XML_Char *version,
    const XML_Char *encoding,
    int standalone
)
{
    //std::cout << "xml: " << version << std::endl;
}


static void StartElementHandler(
    void *iUserData, const XML_Char *iName, const XML_Char **iAtts
)
{
    return ((Expat*)iUserData)->startElementHandler(iName, iAtts);
}


static void EndElementHandler(
    void *iUserData, const XML_Char *iName
)
{
    return ((Expat*)iUserData)->endElementHandler(iName);
}


static void CharacterDataHandler(
    void *iUserData, const XML_Char *iStr, int iLen
)
{
    return ((Expat*)iUserData)->characterDataHandler(iStr, iLen);
}


/**
 * Parser constructor.  Points the callbacks to the static functions;
 * those functions convert the UserData field into the class pointer
 * and pass the callback to the appropriate method.
 */
Expat::Expat()
{
    // Create the parser
    mParser = XML_ParserCreate(0);

    // Set the callbacks to the static functions
    XML_SetUserData(mParser, this);
    XML_SetDefaultHandler(mParser, DefaultHandler);
    XML_SetXmlDeclHandler(mParser, XmlDeclHandler);
    XML_SetStartElementHandler(mParser, StartElementHandler);
    XML_SetEndElementHandler(mParser, EndElementHandler);
    XML_SetCharacterDataHandler(mParser, CharacterDataHandler);
}


Expat::~Expat()
{
    XML_ParserFree(mParser);
}


var Expat::parse(const char* iFile)
{
    std::ifstream is(iFile, std::ifstream::in);
    if (is.fail())
        throw std::runtime_error("xmlfile::read(): Open failed");

    var f;
    XML_Status s;
    while (f.getline(is))
    {
        s = XML_Parse(mParser, f.str(), f.size(), 0);
        if (!s)
            throw std::runtime_error("xmlfile::parse(): Error!");
    }
    s = XML_Parse(mParser, f.str(), 0, 0);
    if (!s)
        throw std::runtime_error("xmlfile::parse(): Error!");
    if (mStack.size() != 0)
        throw std::runtime_error("xmlfile::parse(): Short file?");

    return mVar;
}


var Expat::element()
{
    static var elem;
    if (!elem)
    {
        var nil;
        elem["name"] = nil;
        elem["attr"] = nil;
        elem["data"] = nil;
    }
    return elem.copy();
}


void Expat::startElementHandler(const XML_Char *iName, const XML_Char **iAtts)
{
    var elem = element();
    if (mStack)
        mStack.top()["data"].push(elem);
    else
        mVar = elem;
    mStack.push(elem);
    elem["name"] = iName;
    while (*iAtts)
    {
        elem["attr"][iAtts[0]] = iAtts[1];
        iAtts += 2;
    }
}


void Expat::endElementHandler(const XML_Char *iName)
{
    if (mStack.top()["name"] != iName)
        throw std::runtime_error("Expat::endElementHandler: malformed xml");
    mStack.pop();
}


void Expat::characterDataHandler(const XML_Char *iStr, int iLen)
{
    var str(iLen, iStr);
    mStack.top()["data"].push(str);
}


var Expat::read(const char* iFile)
{
    return parse(iFile);
}

void Expat::write(const char* iFile, var iVar)
{
}
