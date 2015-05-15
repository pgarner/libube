/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2015
 */

#include <unistd.h>

#include "config.h"

using namespace libvar;

Option::Option(int iArgc, char** iArgv, var iOptString)
{
    mArgc = iArgc;
    mArgv = iArgv;
    mOptString = iOptString ? iOptString : "";
    mOpt = -1;
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

ind Option::ind() const
{
    return optind;
}

var Option::arg() const
{
    if (!optarg)
    {
        vstream e;
        e << "at argument " << mOpt << " optarg is null";
        throw error(e);
    }
    return optarg;
};

Option::operator var()
{
    return var(mArgc-optind, mArgv+optind);
};

Config::Config(var iStr)
{
    mStr = iStr.copy();
}

void Config::read(var iConfigFile)
{
    // Config files are .ini format
    vfile ini("ini");
    mCnf = ini.read(iConfigFile);
}

var Config::config()
{
    // If it doesn't exist, this will make the [mStr] entry
    if (mStr)
        return mCnf[mStr];
    return mCnf["Main"];
}
