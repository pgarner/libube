/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

// First so we know the includes in the header are sufficient
#include "lube/ind.h"

#include <iostream>
#include <cassert>

using namespace std;
using namespace libube;

int main(int argc, char** argv)
{
    ind x;
    assert(ind::size());

    // Positive is true and negative is false
    cout << "Initial ind = " << x;
    cout << " which is " << (x ? "True" : "False") << endl;
    x = -1;
    cout << "but now ind = " << x;
    cout << " which is " << (x ? "True" : "False") << endl;
    cout << "Inverts to " << (!x ? "True" : "False") << endl;

    // Arithmetic uses the int context rather than the class
    ind y = 6;
    cout << "y is " << y << "; " << "y+3 is " << y+3 << endl;
    cout << "y is " << y << "; " << "y-7 is " << y-7 << endl;

    // Works in for() loops and operator~() works
    for(ind i=-4; i<5; i++)
        cout << "[" << i << " " << ~i << "]";
    cout << endl;

    // Copies OK
    ind z = y;
    cout << "z is " << z << endl;

    // Done
    return 0;
}
