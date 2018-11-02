/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, March 2015
 */

#include "lube.h"
#include "lube/c++lapack.h"

using namespace std;

int main(int argc, char** argv)
{
    // Set the FP precision to be less than the difference between different
    // numerical libraries
    std::cout.precision(4);

    // This matrix is singular
    var x = lube::range(1.0f, 9.0f).view({3,3});
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

    // Because x is singular, the last eigenvalue is sensitive to numerical
    // precision.  So don't print wr as it varies across implementation.
    // cout << "ret: " << ret << endl //  This seems undefined
    cout << "wr[0]: " << wr[0] << endl;
    cout << "wi: " << wi << endl;
    cout << "vl: " << vl << endl;
    cout << "vr: " << vr << endl;

    x = lube::range(1.0f, 9.0f).view({3,3});
    ret = lapack::gees(
        3, x.ptr<float>(),
        wr.ptr<float>(), wi.ptr<float>()
    );
    cout << "wr[0]: " << wr[0] << endl;
    cout << "wi: " << wi << endl;

    return 0;
}
