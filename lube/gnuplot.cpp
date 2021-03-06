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

#include <lube/module.h>

namespace libube
{
    class gnuplot : public file
    {
    public:
        gnuplot();
        ~gnuplot();
        void puts(const char* iStr);
        virtual var read(var iFile);
        virtual void write(var iFile, var iVar);
    private:
        FILE* mStream;
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new gnuplot;
    }
}


using namespace libube;


gnuplot::gnuplot()
{
    mStream = popen("gnuplot -p", "w");
    if (!mStream)
        throw error("gnuplot: Open failed");
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
        throw error("gnuplot::puts(): Failed to write");
}


var gnuplot::read(var iFile)
{
    throw error("gnuplot::read() Read not defined");
    return var();
}


void gnuplot::write(var iFile, var iVar)
{
    var line;
    if (iFile)
    {
        // Default to whatever gnuplot's default is, but if a file is supplied
        // then write eps to it.
        puts("set term post eps");
        varstream vs;
        vs << "set output " << iFile;
        puts(vs.str());
    }

    // Loop over the input var assuming it's an array of vars
    for (int i=0; i<iVar.size(); i++)
    {
        line = iVar[i];
        switch (line.atype())
        {
        case TYPE_CHAR:
            // It's a string; a gnuplot command
            puts(line.str());
            break;
        case TYPE_INT:
        case TYPE_LONG:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            switch (line.dim())
            {
            case 0:
            case 1:
                // It's a vector of some description; data for the plot
                for (int j=0; j<line.size(); j++)
                {
                    varstream vs;
                    vs << line.at(j);
                    puts(vs.str());
                }
                puts("e");
                break;
            case 2:
            {
                // It's a matrix; write it out as a transpose
                varstream vs;
                for (int j=0; j<line.shape(1); j++)
                {
                    for (int i=0; i<line.shape(0); i++)
                        vs << " " << line(i, j);
                    vs << "\n";
                }
                puts(vs.str());
                puts("e");
                break;
            }
            default:
                throw error("gnuplot::write(): Unknown dimension");
            }
            break;
        default:
            throw error("gnuplot::write(): Unknown data type");
        }
    }
}
