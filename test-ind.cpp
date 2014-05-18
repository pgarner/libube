/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

// First so we know the includes in the header are sufficient
#include "ind.h"

#include <iostream>
#include <cassert>

using namespace std;

int main(int argc, char** argv)
{
    ind x;
    assert(ind::size());
    cout << "Initial ind = " << x;
    cout << " which is " << (x ? "True" : "False") << endl;
    x = -1;
    cout << "but now ind = " << x;
    cout << " which is " << (x ? "True" : "False") << endl;

    ind y = 6;
    cout << "y is " << y << "; " << "y+3 is " << y+3 << endl;
    cout << "y is " << y << "; " << "y-7 is " << y-7 << endl;

    for(ind i=-4; i<5; i++)
        cout << "[" << i << " " << ~i << "]";
    cout << endl;
}
