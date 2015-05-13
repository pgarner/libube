/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2015
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <var.h>

namespace libvar
{
    /**
     * Option class.  A thin wrapper for getopt() from the standard unix
     * library.
     */
    class Option
    {
    public:
        Option(int iArgc, char** iArgv, var iOptString=nil);
        operator bool();
        int get() const { return mOpt; };
        ind ind() const;
        var arg() const;
        operator var();
    private:
        int mArgc;
        char** mArgv;
        var mOptString;
        int mOpt;
    };

    /**
     * Configuration class.  Stores an associative array with attribute
     * information.  The string is used to identify the attributes in, say, an
     * ini file.
     */
    class Config
    {
    public:
        Config(var iStr=nil);
    protected:
        var mAttr;
    private:
        var mStr;
    };
}

#endif // CONFIG_H
