/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2013
 */

#include <cstdio>
#include <stdexcept>

#include <var.h>

namespace libvar
{
    class gnuplot : public varfile
    {
    public:
        gnuplot();
        ~gnuplot();
        void puts(const char* iStr);
        virtual var read(const char* iFile);
        virtual void write(const char* iFile, var iVar);
    private:
        FILE* mStream;
    };
}

using namespace libvar;

void libvar::factory(varfile** oFile)
{
    *oFile = new gnuplot;
}


gnuplot::gnuplot()
{
    mStream = popen("gnuplot", "w");
    if (!mStream)
        throw std::runtime_error("gnuplot: Open failed");
}


gnuplot::~gnuplot()
{
    pclose(mStream);
}


void gnuplot::puts(const char* iStr)
{
    int s = std::fputs(iStr, mStream);
    int n = std::fputs("\n", mStream);
    if (s < 0 || n < 0)
        throw std::runtime_error("gnuplot::puts(): Failed to write");
}


var gnuplot::read(const char* iFile)
{
    throw std::runtime_error("gnuplot::read() Read not defined");
    return var();
}


void gnuplot::write(const char* iFile, var iVar)
{
    var str;
    if (iFile)
    {
        // Default to whatever gnuplot's default is, but if a file is
        // supplied then write eps to it.
        puts("set term post eps");
        str.sprintf("set output \"%s\"", iFile);
        puts(str.str());
    }

    // Loop over the input var assuming it's an array of strings
    for (int i=0; i<iVar.size(); i++)
    {
        str = iVar[i];
        switch (str.atype())
        {
        case TYPE_CHAR:
            puts(str.str());
            break;
        case TYPE_INT:
        case TYPE_LONG:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            for (int j=0; j<str.size(); j++)
            {
                vstream vs;
                vs << str.at(j);
                puts(vs.str());
            }
            puts("e");
            break;
        default:
            throw std::runtime_error("gnuplot::write(): Unknown data type");
        }
    }
}
