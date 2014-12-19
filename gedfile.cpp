/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2013
 */

#include <fstream>
#include <var.h>


namespace libvar
{
    /**
     * Ad-hoc parser for GEDCOM files.
     */
    class GEDCOM : public varfile
    {
    public:
        GEDCOM();
        var loadFile(const char* iFileName);
        virtual var read(const char* iFile);
        virtual void write(const char* iFile, var iVar);

    private:
        var individual();
        var family();
        int readLine();
        bool knownToken(var iToken);
        void doFile(int iLevel);
        int doHeader(int iLevel);
        int doSubmitter(int iLevel);
        int doIndividual(int iLevel, var iInd);
        int doFamily(int iLevel, var iFam);
        int doBirth(int iLevel);
        int doContinue(int iLevel, var iToken);

        var mVar;
        std::ifstream mInStream;
        var mField;

        enum tokenType {
            CONT,
            CONC,
            HEAD,
            TRLR,
            INDI,
            FAM,
            SUBM,
            NAME,
            TITL,
            SEX,
            FAMC,
            FAMS,
            BIRT,
            DEAT,
            CHR,
            BURI,
            PLAC,
            DATE,
            OCCU,
            NOTE,
            HUSB,
            WIFE,
            CHIL,
            MARR,
            DIV,
            REFN
        };
        var mTokenMap;
        var mIndividualMap;
        var mFamilyMap;
    };


    void factory(Module** oModule)
    {
        *oModule = new GEDCOM;
    }
}


using namespace libvar;


GEDCOM::GEDCOM()
{
    // Map strings to enums
    mTokenMap["CONT"] = CONT;
    mTokenMap["CONC"] = CONC;
    mTokenMap["HEAD"] = HEAD;
    mTokenMap["TRLR"] = TRLR;
    mTokenMap["INDI"] = INDI;
    mTokenMap["FAM"]  = FAM;
    mTokenMap["SUBM"] = SUBM;
    mTokenMap["NAME"] = NAME;
    mTokenMap["TITL"] = TITL;
    mTokenMap["SEX"]  = SEX;
    mTokenMap["FAMC"] = FAMC;
    mTokenMap["FAMS"] = FAMS;
    mTokenMap["BIRT"] = BIRT;
    mTokenMap["DEAT"] = DEAT;
    mTokenMap["CHR"]  = CHR;
    mTokenMap["BURI"] = BURI;
    mTokenMap["PLAC"] = PLAC;
    mTokenMap["DATE"] = DATE;
    mTokenMap["OCCU"] = OCCU;
    mTokenMap["NOTE"] = NOTE;
    mTokenMap["HUSB"] = HUSB;
    mTokenMap["WIFE"] = WIFE;
    mTokenMap["CHIL"] = CHIL;
    mTokenMap["MARR"] = MARR;
    mTokenMap["DIV"]  = DIV;
    mTokenMap["REFN"] = REFN;
//    mTokenMap[""] = ;
}


/*
 * Individual constructor
 */
var GEDCOM::individual()
{
    static var ind;
    if (!ind)
    {
        // Build the exemplar
        var nil;
        ind["NAME"] = nil;
        ind["TITL"] = nil;
        ind["SEX"] = nil;
    }
    return ind.copy();
}


/*
 * Family constructor
 */
var GEDCOM::family()
{
    static var fam;
    if (!fam)
    {
        // Build the exemplar
        var nil;
        fam["HUSB"] = nil;
        fam["WIFE"] = nil;
        fam["CHIL"] = nil;
    }
    return fam.copy();
}


int GEDCOM::readLine()
{
    var line;
    if (line.getline(mInStream))
        mField = line.split(" ");
    else
        std::cout << "Premature end of file" << std::endl;
        
    int level = mField.shift().cast<int>();
    //std::cout << "[" << level << "]" << mField << std::endl;
    return level;
}

var GEDCOM::loadFile(const char* iFileName)
{
    mInStream.open(iFileName);
    if (mInStream.fail())
        std::cout << "Open failed" << std::endl;
    doFile(readLine());
    mInStream.close();
    return mVar;
}

bool GEDCOM::knownToken(var iToken)
{
    if (mTokenMap.at(iToken))
        return true;
    std::cout << "Token not in map: " << iToken << std::endl;
    return false;
}

void GEDCOM::doFile(int iLevel)
{
    bool haveTrailer = false;
    int level = iLevel;
    do
    {
        if (mField[0][0] == '@')
        {
            // It's a cross reference
            var xref = mField.shift();
            var token = mField.shift();

            if (!knownToken(token))
                break;

            var entity;
            switch (mTokenMap[token].cast<int>())
            {
            case SUBM:
                doSubmitter(readLine());
                break;
            case INDI:
                entity = individual();
                mIndividualMap[xref] = mVar["Individual"].size();
                mVar["Individual"].push(entity);
                doIndividual(readLine(), entity);
                break;
            case FAM:
                entity = family();
                mFamilyMap[xref] = mVar["Family"].size();
                mVar["Family"].push(entity);
                doFamily(readLine(), entity);
                break;
            default:
                std::cout << "File: Unknown tag " << mField << std::endl;
            }
        }
        else
        {
            // It's a tag
            var token = mField.shift();

            if (!knownToken(token))
                break;
            switch (mTokenMap[token].cast<int>())
            {
            case HEAD:
                level = doHeader(readLine());
                continue;
            case TRLR:
                return;
            default:
                std::cout << "File: Unknown tag " << token << std::endl;
            }
        }
    }
    while (level >= iLevel);
    if (!haveTrailer)
        std::cout << "Trailer not found" << std::endl;
}

int GEDCOM::doHeader(int iLevel)
{
    int level = iLevel;
    do
    {
        level = readLine();
        // Just chuck away the header
    }
     while (level >= iLevel);
    return level;
}

int GEDCOM::doSubmitter(int iLevel)
{
    int level = iLevel;
    do
    {
        level = readLine();
        // Just chuck away the submitter
    }
    while (level >= iLevel);
    return level;
}

int GEDCOM::doIndividual(int iLevel, var iInd)
{
    int level = iLevel;
    do
    {
        var token = mField.shift();
        if (!knownToken(token))
            break;
        switch (mTokenMap[token].cast<int>())
        {
        case NAME:
            iInd["NAME"] = mField.join(" ");
            break;
        case TITL:
            iInd["TITL"] = mField.join(" ");
            break;
        case SEX:
            iInd["SEX"] = mField.join(" ");
            break;
        case BIRT:
        case DEAT:
        case CHR:
        case BURI:
            level = readLine();
            if (level > iLevel)
                level = doBirth(level);
            continue;
        case OCCU:
            std::cout << "Occupation" << std::endl;
            break;
        case CONT:
        case CONC:
            level = doContinue(level, token);
            continue;
        case NOTE:
            std::cout << "Note" << std::endl;
            break;
        case FAMS: // Family where this person is a spouse or parent
        case FAMC: // Family where this person is a child
            // No need to do anything as the family already encodes it
            break;
        default:
            std::cout << "Individual: Unknown tag" << mField << std::endl;
        }
        level = readLine();
    }
    while (level >= iLevel);
    return level;
}

int GEDCOM::doBirth(int iLevel)
{
    std::cout << "Birth / Death ";
    int level = iLevel;
    do
    {
        var token = mField.shift();
        if (!knownToken(token))
            break;
        switch (mTokenMap[token].cast<int>())
        {
        case DATE:
            std::cout << "Date" << std::endl;
            break;
        case PLAC:
            std::cout << "Place" << std::endl;
            break;
        case CONT:
        case CONC:
            level = doContinue(level, token);
            continue;
        case NOTE:
            std::cout << "Note" << std::endl;
            break;
        default:
            std::cout << "Birth: Unknown tag" << token << std::endl;
        }
        level = readLine();
    }
    while (level >= iLevel);
    return level;
}

int GEDCOM::doFamily(int iLevel, var iFam)
{
    int level = iLevel;

    // Read family
    do
    {
        var token = mField.shift();
        var indiv;
        if (!knownToken(token))
            break;
        switch (mTokenMap[token].cast<int>())
        {
        case HUSB:
            indiv = mField.shift();
            if (!mIndividualMap.at(indiv))
                std::cout << "Missing: " << indiv << std::endl;
            else
                iFam["HUSB"] = mIndividualMap.at(indiv).cast<int>();
            break;
        case WIFE:
            indiv = mField.shift();
            if (!mIndividualMap.at(indiv))
                std::cout << "Missing: " << indiv << std::endl;
            else
                iFam["WIFE"] = mIndividualMap.at(indiv).cast<int>();
            break;
        case CHIL:
            indiv = mField.shift();
            if (!mIndividualMap.at(indiv))
                std::cout << "Missing: " << indiv << std::endl;
            else
                iFam["CHIL"].push(mIndividualMap.at(indiv).cast<int>());
            break;
        case DIV:
            std::cout << "Div ?" << std::endl;
            break;
        case MARR:
            std::cout << "Married" << std::endl;
            level = doBirth(readLine());
            continue;
        default:
            std::cout << "Family: Unknown tag" << token << std::endl;
        }
        level = readLine();
    }
    while (level >= iLevel);

    // Done
    return level;
}

int GEDCOM::doContinue(int iLevel, var iToken)
{
    int level = iLevel;
    do
    {
        std::cout << "Continue" << std::endl;
        switch (mTokenMap[iToken].cast<int>())
        {
        case CONT:
            std::cout << "Continue" << std::endl;
            break;
        case CONC:
            std::cout << "Concatenate" << std::endl;
            break;
        default:
            std::cout << "Continue: Unknown tag" << iToken << std::endl;
        }
        level = readLine();
    }
    while (level >= iLevel);
    return level;
}


var GEDCOM::read(const char* iFile)
{
    return loadFile(iFile);
}


void GEDCOM::write(const char* iFile, var iVar)
{
}
