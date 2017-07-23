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

var Option::args()
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
        << mName.str() << "\n" << "Usage: " << mArgv[0];
    if (mOptString != "")
        std::cout << " -[" << mOptString.str() << "]";
    std::cout << " [args] (see 'man 3 getopt')\n";
    for (int i=0; i<mUsage.size(); i++)
        std::cout << mUsage[i].str() << "\n";
}

/**
 * Static config.  This functions as a global config that gets used if no other
 * is specified.
 */
namespace libube
{
    static var sConfig;
}

/**
 * Config constructor
 *
 * For the moment it uses a static (global) config.  It would be possible to
 * have a local one too, but it's for the future.
 */
Config::Config(var iSection)
{
    // Make sure sConfig is an associative array so we copy the reference.  OK,
    // this is only relevant if we have local configs.
    if (!sConfig)
        sConfig[nil];
    configSection(iSection);
}

void Config::configFile(var iConfigFile)
{
    // Config files are .ini format
    filemodule im("ini");
    file& ini = im.create();
    var cnf = ini.read(iConfigFile);

    // Merge the new config into the current one
    for (int i=0; i<cnf.size(); i++)
    {
        var seckey = cnf.key(i);
        var secval = cnf[i];
        for (int j=0; j<secval.size(); j++)
        {
            var entry = secval.key(j);
            sConfig[seckey][entry] = secval[entry];
        }
    }
}

var Config::configSection(var iSection)
{
    if (!iSection)
        mSection = nil;
    else
        mSection = iSection;
    return sConfig[mSection];
}

var Config::config(var iEntry)
{
    var sec = sConfig[mSection];
    if (sec && sec.index(iEntry))
        return sec[iEntry];
    return nil;
}

void Config::configSet(var iEntry, var iValue)
{
    if (!mSection)
        return;
    sConfig[mSection][iEntry] = iValue;
}

template<> const char* Config::config(var iEntry, const char* iDefault)
{
    var sec = sConfig[mSection];
    if (sec && sec.index(iEntry))
        return sec[iEntry].str();
    return iDefault;
}

#define CONFIG(T)                                                \
    template<> T Config::config<T>(var iEntry, T iDefault)       \
    {                                                            \
        var sec = sConfig[mSection];                             \
        if (sec && sec.index(iEntry))                            \
            return sec[iEntry].cast<T>();                        \
        return iDefault;                                         \
    }

CONFIG(int)
CONFIG(long)
CONFIG(float)
CONFIG(double)
