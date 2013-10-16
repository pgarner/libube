/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2013
 */

#include <var>
#include <fstream>
#include <stdexcept>

extern "C" {
    void read(const char* iFile, var& iVar);
    void write(const char* iFile, var iVar);
}

void read(const char* iFile, var& iVar)
{
    std::ifstream is(iFile, std::ifstream::in);
    if (is.fail())
        throw std::runtime_error("txtfile::read(): Open failed");
    var f;
    iVar.clear();
    while (f.getline(is).defined())
        iVar.push(f.copy());
}

void write(const char* iFile, var iVar)
{
    std::ofstream os(iFile, std::ofstream::out);
    if (os.fail())
        throw std::runtime_error("txtfile::write(): Open failed");
    for (int i=0; i<iVar.size(); i++)
        os << iVar.at(i) << std::endl;
}
