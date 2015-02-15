/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, February 2015
 */

#include "lv.h"

using namespace std;

void function()
{
    cout << "Throwing an exception" << endl;
    throw lv::vruntime_error("This is the exception");
}

int main()
{
    cout << "Calling a function" << endl;
    function();
    return 0;
}
