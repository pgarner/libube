/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, March 2015
 */

#include "lv.h"
#include "c++lapack.h"

using namespace std;

int main(int argc, char** argv)
{
    // This matrix is singular
    var x = lv::range(1.0f, 9.0f).view({3,3});
    cout << "x is: " << x << endl;

    var vl = var(9, 0.0f).view({3,3});
    var vr = var(9, 0.0f).view({3,3});
    var wr(3, 0.0f);
    var wi(3, 0.0f);
    float *z = 0;
    long ret = lapack::geev(
        3, x.ptr<float>(),
        wr.ptr<float>(), wi.ptr<float>(),
        z, vr.ptr<float>()
    );
    cout << "ret: " << ret << endl;
    cout << "wr: " << wr << endl;
    cout << "wi: " << wi << endl;
    cout << "vl: " << vl << endl;
    cout << "vr: " << vr << endl;

    return 0;
}
