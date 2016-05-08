#include <fstream>
#include <lube.h>

using namespace std;

void write(const char* iFile, var iVar)
{
    cout << "Writing: " << iVar << endl;
    ofstream os(iFile, ofstream::out);
    if (os.fail())
        cout << "Open failed" << endl;
    os << iVar << endl;
}

var read(const char* iFile)
{
    ifstream is(iFile, ifstream::in);
    if (is.fail())
        cout << "Open failed" << endl;
    var in;
    is >> in;
    cout << "Read: " << in << endl;
    return in;
}

int main(int argc, char** argv)
{
    var o1 = "This is just a string";
    write("test1.json", o1);
    var i1 = read("test1.json");

    var o2;
    o2[0] = "Zero";
    o2[1] = "One";
    write("test2.json", o2);
    var i2 = read("test2.json");

    var o3;
    o3["zero"] = "Zero";
    o3["one"] = "One";
    write("test3.json", o3);
    var i3 = read("test3.json");

    var o4;
    o4[0] = o2;
    o4[1] = o3;
    write("test4.json", o4);
    var i4 = read("test4.json");

    var o5;
    o5["first"] = o2;
    o5["second"] = o3;
    write("test5.json", o5);
    var i5 = read("test5.json");
}
