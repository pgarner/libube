/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2013
 */

#include <var>
#include <cstdio>
#include <stdexcept>

#include "varfile.h"


class gnuplot
{
public:
    gnuplot();
    ~gnuplot();
    void puts(const char* iStr);
private:
    FILE* mStream;
};


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


void read(const char* iFile, var& oVar)
{
    throw std::runtime_error("gnuplot::read() Read not defined");
}


void write(const char* iFile, var iVar)
{
    gnuplot gp;
    var str;
    if (iFile)
    {
        // Default to whatever gnuplot's default is, but if a file is
        // supplied then write eps to it.
        gp.puts("set term post eps");
        str.sprintf("set output \"%s\"", iFile);
        gp.puts(&str);
    }

    // Loop over the input var assuming it's an array of strings
    for (int i=0; i<iVar.size(); i++)
    {
        str = iVar[i];
        switch (str.type())
        {
        case var::TYPE_CHAR:
            gp.puts(&str);
            break;
        case var::TYPE_INT:
        case var::TYPE_LONG:
        case var::TYPE_FLOAT:
        case var::TYPE_DOUBLE:
            for (int j=0; j<str.size(); j++)
            {
                vstream vs;
                vs << str.at(j);
                gp.puts(&vs);
            }
            gp.puts("e");
            break;
        default:
            throw std::runtime_error("gnuplot::write(): Unknown data type");
        }
    }
}
