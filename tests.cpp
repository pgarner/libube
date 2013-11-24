#include <cassert>
#include <var>
#include <map>
#include <fstream>

using namespace std;

var getString()
{
    cout << "In getString()" << endl;
    var str;
    str = "An example string with spaces.";
    return str;
}

void useString(var s)
{
    // The ampersand suppresses the double quotes by passing the raw
    // char* to the format operator.
    cout << "Using: " << &s << endl;
}

int main(int argc, char** argv)
{
    // Copy the command line
    var arg(argc, argv);

    // Check that one and two level indexing works
    cout << "Arg: " << arg << endl;
    cout << "Arg[0]: " << arg[0] << endl;
    cout << "Arg[0][0]: " << arg[0][0] << endl;

    // Check that operator == works
    if (arg[0][0] == '.')
        cout << "It's a dot" << endl;
    else
        cout << "It's not a dot" << endl;

    // Check that index() finds (or not) an element
    if (arg.index("-f").defined())
        cout << "There's a -f" << endl;
    else
        cout << "There's no -f" << endl;

    // Basic numerical tests
    var s, w, x, y, z, dummy;
    w = 'w';
    x = 2;
    y = 3.14;
    z = y.cos();
    s = "Hello!";

    cout << "Var size is " << sizeof(var) << endl;
    cout << w << endl;
    cout << x << endl;
    cout << y << endl;
    cout << z << endl;
    cout << s << endl;
    cout << s.at(0) << endl;
    cout << s[0] << endl;

    // Numerical overloads
    x += 2;
    y += 3.14f;
    x.push(2);
    y -= 1;
    cout << x << endl;
    cout << y << endl;

    // Reference the same object & reference counting
    var a = s;
    cout << "a is: " << a << endl;
    a = "New string";

    // String comparison
    var n = "New string";
    if (a == n)
        cout << "equal" << endl;
    else
        cout << "not equal" << endl;

    // Pass a string into a function, it's still valid outside
    cout << "Calling getString()" << endl;
    var b = getString();
    cout << "b is: " << b << endl;
    useString(b);
    cout << "b is: " << b << endl;

    // Basic string split
    var sp;
    sp = b.split("n");
    cout << "sp is: " << sp << endl;

    // String strip
    var ss = "  Hello ";
    cout << ss;
    cout << " strips to ";
    cout << ss.strip() << endl;

    // Basic string insert
    a.insert("ddd", 1);
    cout << "a is: " << a << endl;
    a.append("aaa");
    cout << "a is: " << a << endl;

    // String sprintf
    var str;
    cout << "sprintf: " << str.sprintf("This string is %d %f", 1, 0.1) << endl;

    // Cast to char* and back
    y.cast<char*>();
    cout << "y is: " << y << endl;
    y.cast<float>();
    cout << "y is: " << y << endl;

    // Shifting of command line
    arg.insert("insert", 1);
    cout << "arg is: " << arg << endl;
    arg.remove(0);
    cout << "arg is: " << arg << endl;
    var as = arg.shift();
    cout << "arg is: " << arg << " shifted: " << as << endl;

    // Join the command line args
    cout << "Joining: " << arg << endl;
    cout << "Joined: " << arg.join("-") << endl;

    // Sort the command line args
    cout << arg.sort() << endl;

    // Check that we can act as a std::map key (just needs operator<)
    map<var, int> map;
    map["One"] = 1;
    map["Two"] = 2;
    map["Three"] = 3;
    cout << map.count("Zero") << endl;
    cout << map.count("One") << endl;

    // Read a small file
    var f;
    var t;
    ifstream is("tests.txt", ifstream::in);
    if (is.fail())
        cout << "Open failed" << endl;
    while (f.getline(is).defined())
    {
        cout << f << endl;
        t.push(f.copy());
    }
    cout << t << endl;

    // Shallow copy
    var c1 = arg;
    c1.push("extra");
    cout << c1 << endl;
    cout << arg << endl;
    var c2 = arg.copy();
    c2.pop();
    cout << c2 << endl;
    cout << arg << endl;

    // Modify an array using operator[]
    int xxa[] = {1,2,3,4,5};
    var xa(5, xxa);
    cout << "xa is " << xa << endl;
    xa[1] = xa[0];
    xa[2] += xa[1];
    xa[3] = 7;
    xa[4] += 7;
    cout << "xa is " << xa << endl;

    // Array of vars by pushing
    var arr1;
    arr1.push("Hi!");
    cout << arr1 << endl;

    // Array of vars just by assigning indeces
    var arr2;
    arr2[1] = "Hi!";
    cout << arr2 << endl;

    // Array indexed by var
    var vmap;
    vmap["one"] = 1;
    vmap["two"] = 2;
    vmap["three"] = 3;
    cout << "vmap[0] is " << vmap[0] << endl;
    cout << "vmap[1] is " << vmap[1] << endl;
    cout << "vmap[\"three\"] is " << vmap["three"] << endl;
    cout << "vmap is " << vmap << endl;
    cout << "vmap.copy() is " << vmap.copy() << endl;

    // Multi-dimensional array
    var iarr;
    iarr[1][2] = 3;
    iarr[1][4] = 5;
    cout << "iarr is " << iarr << endl;

    // Multi-dimensional map
    var wmap;
    wmap["one"]["two"] = "three";
    wmap["one"]["four"] = "five";
    cout << "wmap is " << wmap << endl;

    // Init from variable argument list
    var ai(5, 5,4,3,2,1);
    cout << "ai is: " << ai << endl;

    // Read a text file via a dynamic library
    var txt;
    txt.read("tests.txt", "txt");
    cout << "Loaded: " << txt << endl;

    // Read a .ini file via a dynamic library
    var ini;
    ini.read("tests.ini", "ini");
    cout << "Loaded: " << ini << endl;

    // Plot something with gnuplot
    var gnu;
    gnu.push("plot sin(x), \"-\"");
    gnu.push(ai);
    gnu.write("tests.eps", "gnuplot");

    // Tensors
    var ts = 0.0f;
    ts.resize(16);
    for (int i=0; i<16; i++)
        ts[i] = (float)i;
    var t1 = ts.view(2, 4, 4);
    var t2 = ts.view(3, 2, 2, 4);
    cout << ts << endl;
    cout << t1 << endl;
    cout << t2 << endl;
    cout << "t1(1,2): " << t1(1,2) << endl;
    cout << "t2(1,1,2): " << t2(1,1,2) << endl;
    t1(1,2) = 2.3f;
    cout << t1 << endl;

    // BLAS
    var bt(4, 1.0f, 1.2, 0.8, -2.0);
    cout << bt << "  sums to " << bt.sum()  << endl;
    cout << bt << " asums to " << bt.asum() << endl;

    // Stream
    vstream vstr;
    float fl = 23e-1f;
    vstr << "H";
    cout << vstr.var() << endl;
    vstr << "ello: " << fl;
    cout << vstr.var() << endl;

    // gedcom
    var ged;
    ged.read("tests.ged", "ged");
    cout << "Loaded: " << ged << endl;

    return 0;
}
