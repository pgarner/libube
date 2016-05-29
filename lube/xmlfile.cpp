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
#include <lube/var.h>
#include <lube/data.h>

#include <expat.h>


namespace libube
{
    /**
     * The main file handler class
     */
    class XMLFile : public file
    {
    public:
        virtual var read(var iFile);
        virtual void write(var iFile, var iVar);
    };


    /**
     * Class to wrap the expat library and callbacks.
     */
    class Expat
    {
    public:
        Expat();
        ~Expat();
        var parse(const char* iFile);
        void startElementHandler(const XML_Char *iName, const XML_Char **iAtts);
        void endElementHandler(const XML_Char *iName);
        void characterDataHandler(const XML_Char *iStr, int iLen);
    private:
        var mVar;            ///< Var to populate during parse
        var mStack;          ///< Parse stack
        XML_Parser mParser;  ///< The expat parser
        var element();
    };

    /**
     * XML writer class
     */
    class XMLWriter
    {
    public:
        void write(const char* iFile, var iVar);
    private:
        bool writeElem(std::ofstream& iOS, var iVar);
        void escape(std::ofstream& iOS, var iVar);
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new XMLFile;
    }
}


using namespace libube;


var XMLFile::read(var iFile)
{
    // Instantiate an expat class and use it to parse the file
    Expat expat;
    return expat.parse(iFile.str());
}

void XMLFile::write(var iFile, var iVar)
{
    // Instantiate an XML writer and write the file
    XMLWriter writer;
    writer.write(iFile.str(), iVar);
}


/**
 * In principle this is unnecessary; in practice it's useful to catch
 * errors (of the form "dude, you forgot to implement...").
 */
static void DefaultHandler(
    void *iUserData, const XML_Char *iStr, int iLen
)
{
    // DOS line ends find their way here.  Why?
    if ((iLen == 1) && (*iStr == '\r'))
        return;

    var str(iLen, iStr);
    std::cout << "expat::DefaultHandler: " << str << std::endl;
    throw error("xmlfile: default handler called");
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
        throw error("xmlfile::read(): Open failed");

    var f;
    XML_Status s;
    while (f.getline(is))
    {
        s = XML_Parse(mParser, f.str(), f.size(), 0);
        if (!s)
            throw error("xmlfile::parse(): Error!");
    }
    s = XML_Parse(mParser, f.str(), 0, 0);
    if (!s)
        throw error("xmlfile::parse(): Error!");
    if (mStack.size() != 0)
        throw error("xmlfile::parse(): Short file?");

    return mVar;
}


var Expat::element()
{
    static var elem;
    if (!elem)
    {
        // DATA first means it gets allocated in one block
        elem[DATA] = nil;
        elem[NAME] = nil;
        elem[ATTR] = nil;
        elem[TYPE] = "text";
    }
    return elem.copy();
}


void Expat::startElementHandler(const XML_Char *iName, const XML_Char **iAtts)
{
    var elem = element();
    if (mStack)
        mStack.top()[DATA].push(elem);
    else
        mVar = elem;
    mStack.push(elem);
    elem[NAME] = iName;
    while (*iAtts)
    {
        elem[ATTR][iAtts[0]] = iAtts[1];
        iAtts += 2;
    }
}


void Expat::endElementHandler(const XML_Char *iName)
{
    if (mStack.top()[NAME] != iName)
        throw error("Expat::endElementHandler: malformed xml");
    mStack.pop();
}


void Expat::characterDataHandler(const XML_Char *iStr, int iLen)
{
    var str(iLen, iStr);
    mStack.top()[DATA].push(str);
}


void XMLWriter::write(const char* iFile, var iVar)
{
    // Output stream
    std::ofstream os(iFile, std::ofstream::out);
    if (os.fail())
        throw error("xmlwriter::write(): Open failed");

    // XML declaration
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;

    // Loop over iVar
    if (!writeElem(os, iVar))
        throw error("xmlwriter::write(): Top level not element");
    os << std::endl;
}

bool XMLWriter::writeElem(std::ofstream& iOS, var iVar)
{
    // Exit if it's not an element
    if ((iVar.size() != 4) || (iVar[TYPE] != "text"))
        return false;

    // It's an element
    var name = iVar[NAME];
    var attr = iVar[ATTR];
    var data = iVar[DATA];
    iOS << "<" << name.str();
    if (attr)
        for (int i=0; i<attr.size(); i++)
        {
            iOS << " " << attr.key(i).str() << "=\"";
            escape(iOS, attr[i]);
            iOS << "\"";
        }
    if (!data)
    {
        iOS << " />";
        return true;
    }
    iOS << ">";
    for (int i=0; i<data.size(); i++)
        if (!writeElem(iOS, data[i]))
            escape(iOS, data[i]);
    iOS << "</" << name.str() << ">";
    return true;
}

void XMLWriter::escape(std::ofstream& iOS, var iVar)
{
    const char* str = iVar.str();
    for (int i=0; i<iVar.size(); i++)
        switch (str[i])
        {
        case '&':
            iOS << "&amp;";
            break;
        case '<':
            iOS << "&lt;";
            break;
        case '>':
            iOS << "&gt;";
            break;
        case '\'':
            iOS << "&apos;";
            break;
        case '"':
            iOS << "&quot;";
            break;
        default:
            iOS.put(str[i]);
            break;
        }
}
