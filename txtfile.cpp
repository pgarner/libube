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
    class txtfile : public varfile
    {
    public:
        virtual var read(const char* iFile);
        virtual void write(const char* iFile, var iVar);
    };

    void factory(Module** oModule)
    {
        *oModule = new txtfile;
    }
}


using namespace libvar;


var txtfile::read(const char* iFile)
{
    std::ifstream is(iFile, std::ifstream::in);
    if (is.fail())
        throw std::runtime_error("txtfile::read(): Open failed");

    var o;
    var f;
    while (f.getline(is))
        o.push(f.copy());
    return o;
}

void txtfile::write(const char* iFile, var iVar)
{
    std::ofstream os(iFile, std::ofstream::out);
    if (os.fail())
        throw std::runtime_error("txtfile::write(): Open failed");
    for (int i=0; i<iVar.size(); i++)
        os << iVar.at(i) << std::endl;
}
