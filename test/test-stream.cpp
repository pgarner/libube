#include <cassert>
#include <map>

#include "lube.h"

using namespace std;

int main(int argc, char** argv)
{
    // Stream
    varstream vstr;
    float fl = 23e-1f;
    vstr << "H";
    cout << var(vstr) << endl;
    vstr << "ello: " << fl;
    cout << var(vstr) << endl;

    // varstream as std::iostream
    varstream iostr("1");
    iostr.exceptions(ios_base::badbit | ios_base::failbit);
    iostr << " 2 3.4";
    iostr << " end";
    cout << var(iostr) << endl;

    // operator>> parses according to the type of the operand
    int ioi;
    float iof;
    iostr >> ioi;
    cout << var(iostr) << endl;
    cout << ioi << endl;
    iostr >> ioi;
    cout << var(iostr) << endl;
    cout << ioi << endl;
    iostr >> iof;
    cout << var(iostr) << endl;
    cout << iof << endl;

    // Done
    return 0;
}
