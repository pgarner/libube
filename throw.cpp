/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, February 2015
 */

#include "lube.h"

using namespace std;

void myfunction()
{
    cout << "Throwing an exception" << endl;
    throw lube::error("This is the exception");
}

int main()
{
    cout << "Calling a function" << endl;
    myfunction();
    return 0;
}
