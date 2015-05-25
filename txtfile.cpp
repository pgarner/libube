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

namespace libvar
{
    class txtfile : public File
    {
    public:
        virtual var read(var iFile);
        virtual void write(var iFile, var iVar);
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new txtfile;
    }
}


using namespace libvar;


var txtfile::read(var iFile)
{
    std::ifstream is(iFile.str(), std::ifstream::in);
    if (is.fail())
        throw error("txtfile::read(): Open failed");

    var o;
    var f;
    while (f.getline(is))
        o.push(f.copy());
    return o;
}

void txtfile::write(var iFile, var iVar)
{
    std::ofstream os(iFile.str(), std::ofstream::out);
    if (os.fail())
        throw error("txtfile::write(): Open failed");
    for (int i=0; i<iVar.size(); i++)
        os << iVar.at(i) << std::endl;
}
