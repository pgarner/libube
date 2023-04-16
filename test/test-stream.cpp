#include <cassert>
#include <map>

#include "lube/lube.h"

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

    // Stream should point to " end".  Check that unget() works
    iostr.get();          // ' ' 
    char a = iostr.get(); // 'e'
    cout << "get(): " << a << endl;
    iostr.unget();
    char b = iostr.get();
    cout << "unget(): " << b << endl;

    // Clear the flags (there aren't any)
    iostr.clear();

    // Check stream positioning
    double d;
    iostr.seekg(2);
    iostr >> d;
    cout << "Pos 2: " << d << endl;
    iostr.seekg(0);
    iostr >> d;
    cout << "Pos 0: " << d << endl;
    iostr.seekg(4);
    iostr >> d;
    cout << "Pos 4: " << d << endl;

    // Done
    return 0;
}
