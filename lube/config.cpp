/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2015
 */

#include <unistd.h>

#include "lube/module.h"
#include "lube/config.h"

using namespace libube;

Option::Option()
{
    optind = 1; // Resets getopt.  This class should be a singleton.
    mArgc = 0;
    mArgv = 0;
    mOptString = "";
    mOpt = -1;
}

Option::Option(int iArgc, char** iArgv, var iOptString) : Option()
{
    mArgc = iArgc;
    mArgv = iArgv;
    mOptString = iOptString ? iOptString : "";
}

Option::Option(var iName) : Option()
{
    mName = iName;
}

/**
 * A kind of trick: The getopt() call is issued by evaluating the class in a
 * boolean context.  This is basically a while (opt) call, which is pretty much
 * what you want.  The expression to put in the switch() is then opt.get(); no
 * temporary variables should be needed.
 */
Option::operator bool()
{
    // Should be const, but .str() isn't, so something is wrong.
    mOpt = getopt(mArgc, mArgv, mOptString.str());
    return mOpt != -1;
}

// Could we just evaluate the class in an ind context?
ind Option::index() const
{
    return optind;
}

var Option::arg() const
{
    if (!optarg)
    {
        varstream e;
        e << "at argument " << mOpt << " optarg is null";
        throw error(e);
    }
    return optarg;
};

Option::operator var()
{
    return var(mArgc-optind, mArgv+optind);
};

void Option::operator ()(char iChar, var iDescription, var iDefault)
{
    mOpts[var(iChar)] = iDefault;
    mOptString.push(iChar);
    if (iDefault)
        mOptString.push(':');
    varstream desc;
    desc << " -" << iChar << " " << iDescription.str();
    if (iDefault)
        desc << " [" << iDefault << "]";
    mUsage.push(var(desc));
}

void Option::operator ()(var iDescription)
{
    mUsage.push(iDescription.str());
}

var Option::parse(int iArgc, char** iArgv)
{
    mArgc = iArgc;
    mArgv = iArgv;
    while (*this)
    {
        var o = (char)get();
        if (o == '?')
            usage(0);
        var a = mOpts[o] ? arg() : 1;
        mOpts[o] = a;
    }
    return mOpts;
}

void Option::usage()
{
    std::cout
        << mName.str() << "\n"
        << "Usage: " << mArgv[0]
        << " -[" << mOptString.str() << "] [args] (see 'man 3 getopt')\n";
    for (int i=0; i<mUsage.size(); i++)
        std::cout << mUsage[i].str() << "\n";
}

Config::Config(var iStr)
{
    mStr = iStr.copy();
}

void Config::read(var iConfigFile)
{
    // Config files are .ini format
    filemodule im("ini");
    file& ini = im.create();
    mCnf = ini.read(iConfigFile);
}

var Config::config()
{
    // If it doesn't exist, this will make the [mStr] entry
    if (mStr)
        return mCnf[mStr];
    return mCnf["Main"];
}
