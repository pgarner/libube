/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2015
 */

// First so we know the includes in the header are sufficient
#include "lube/config.h"

#include "lube.h"

using namespace std;

class Configurable : public lube::Config
{
public:
    Configurable(var iStr="Test")
        : Config(iStr)
    {
    };

    void dump()
    {
        std::cout << "Dump of test section: "
                  << configSection("Test") << std::endl;
    };
};

int main(int argc, char** argv)
{
    // Use Option to parse the command line
    lube::Option opt(argc, argv, "abc:");
    while(opt)
        switch (opt.get())
        {
        case 'a':
            cout << "Option A at index " << opt.index() << endl;
            break;
        case 'b':
            cout << "Option B at index " << opt.index() << endl;
            break;
        case 'c':
            cout << "Option C at index " << opt.index()
                 << " is " << opt.arg() << endl;
            break;
        default:
            cout << "Unrecognised option" << endl;
        }

    // The remaining opts are in context var
    var arg = opt.args();
    cout << "Remaining opts: " << arg << endl;

    // The other Option interface
    lube::Option o("Option testing program");
    o('a', "Indicates that an a is pertinent");
    o('b', "Similar to a, later in alphabet");
    o('c', "An option with an argument", 3.14f);
    o('f', "Skip a few; check it doesn't infer -d or -e");
    var opts = o.parse(argc, argv);
    cout << "Options " << opts << endl;
    var args = o.args();
    cout << "Remaining opts: " << args << endl;

    // A new configurable thing
    Configurable conf;
    conf.configFile(TEST_DIR "/test-config.cnf");

    // Specific config values
    cout << "Integer as var is " << conf.config("integer") << endl;
    cout << "   ...default 100 " << conf.config("integer", 100) << endl;
    cout << "Double as var is  " << conf.config("double") << endl;
    cout << "   ...default 100 " << conf.config("double", 100.0) << endl;

    // Dump to check we didn't introduce any more
    conf.dump();

    // Done
    return 0;
}
