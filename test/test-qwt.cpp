/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2016
 */

#include "lube/lube.h"
#include "lube/qwt.h"

using namespace lube;
using namespace std;

int main(int argc, char** argv)
{
    // Start with something to plot
    var r = range(0.0, 3.14*2, 0.05);
    var s = lube::sin(r);
    var c = lube::cos(r);
    
    // Instantiate a plot window
    qwtmodule qm;
    qwt& q = qm.create();
    q.plot();
    q.curve(r, s, "Sine");
    q.curve(r, c, "Cosine");
    q.axes("Abscissa", "Ordinate");
    q.exec();
}
