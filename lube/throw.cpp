/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, February 2015
 */

#include "var.h"

using namespace libube;
using namespace std;

void myfunction()
{
    cout << "Throwing an exception" << endl;
    throw error("This is the exception");
}

/**
 * Exceptions either crash (right now on mac) or produce different stack traces
 * (Debian vs. Arch), so they're not very good for test cases.
 */
int main()
{
    // Exception
    var ts = "Exception string";
    try {
        throw error(ts);
    }
    catch (error e) {
        cout << "Caught: " << e.what() << endl;
    };

    // Throw in a function
    cout << "Calling a function" << endl;
    myfunction();
    return 0;
}
