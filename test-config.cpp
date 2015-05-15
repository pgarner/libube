/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2015
 */

// First so we know the includes in the header are sufficient
#include "config.h"

#include "lv.h"

using namespace std;

class Configurable : public lv::Config
{
public:
    Configurable(lv::Config& iConfig, var iStr="Test")
        : Config(iConfig)
    {
        mStr = iStr;
    };

    void dump() { std::cout << config() << std::endl; };
};

int main(int argc, char** argv)
{
    // Use Option to parse the command line
    lv::Option opt(argc, argv, "abc:");
    while(opt)
        switch (opt.get())
        {
        case 'a':
            cout << "Option A " << opt.ind() << endl;
            break;
        case 'b':
            cout << "Option B " << opt.ind() << endl;
            break;
        case 'c':
            cout << "Option C " << opt.ind() << endl;
            break;
        default:
            cout << "Unrecognised option" << endl;
        }

    // The remaining opts are in context var
    var arg = opt;
    cout << "Remaining opts: " << arg << endl;

    // Load a config file
    lv::Config cnf;
    cnf.read("test-config.cnf");

    // A new configurable thing
    Configurable conf(cnf);
    conf.dump();

    // Done
    return 0;
}
