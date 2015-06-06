/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2013
 */

#include <fstream>
#include <stdexcept>
#include <lube/var.h>

namespace libube
{
    class inifile : public File
    {
    public:
        virtual var read(var iFile);
        virtual void write(var iFile, var iVar);
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new inifile;
    }
}


using namespace libube;


var inifile::read(var iFile)
{
    std::ifstream is(iFile.str(), std::ifstream::in);
    if (is.fail())
        throw error("inifile::read(): Open failed");

    var oVar;
    var f;
    var section = "";
    while (f.getline(is))
    {
        f.strip();
        if (f.size() == 0 || f[0] == ';' || f[0] == '#')
            // It's blank or a comment line
            continue;
        if (f[0] == '[')
        {
            // It's a section line
            ind close = f.index(']');
            if (!close)
                throw error("inifile::read(): can't find ]");
            section = f.copy();
            section.resize(close);
            section.shift();
        }
        else
        {
            // It's a 'key [=:] value' line
            var kv = f.split("=");
            if (kv.size() < 2)
                kv = f.split(":");
            if (kv.size() < 2)
                throw error("inifile::read(): couldn't split");
            kv[0].strip();
            kv[1].strip();
            oVar[section][kv[0]] = kv[1];
        }
    }
    return oVar;
}

void inifile::write(var iFile, var iVar)
{
    std::ofstream os(iFile.str(), std::ofstream::out);
    if (os.fail())
        throw error("inifile::write(): Open failed");
    for (int i=0; i<iVar.size(); i++)
        os << iVar.at(i) << std::endl;
}
