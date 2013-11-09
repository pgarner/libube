/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2013
 */

#include <var>
#include <fstream>
#include <stdexcept>

#include "varfile.h"


void read(const char* iFile, var& oVar)
{
    std::ifstream is(iFile, std::ifstream::in);
    if (is.fail())
        throw std::runtime_error("inifile::read(): Open failed");

    oVar.clear();
    var f;
    var section = "";
    while (f.getline(is).defined())
    {
        f.strip();
        if (f.size() == 0 || f[0] == ';' || f[0] == '#')
            // It's blank or a comment line
            continue;
        if (f[0] == '[')
        {
            // It's a section line
            var close = f.index(']');
            if (!close.defined())
                throw std::runtime_error("inifile::read(): can't find ]");
            section = f.copy();
            section.resize(close.cast<int>());
            section.shift();
        }
        else
        {
            // It's a 'key [=:] value' line
            var kv = f.split("=");
            if (kv.size() < 2)
                kv = f.split(":");
            if (kv.size() < 2)
                throw std::runtime_error("inifile::read(): couldn't split");
            kv[0].strip();
            kv[1].strip();
            oVar[section][kv[0]] = kv[1];
        }
    }
}

void write(const char* iFile, var iVar)
{
    std::ofstream os(iFile, std::ofstream::out);
    if (os.fail())
        throw std::runtime_error("inifile::write(): Open failed");
    for (int i=0; i<iVar.size(); i++)
        os << iVar.at(i) << std::endl;
}
