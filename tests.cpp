#include <cassert>
#include <map>
#include <fstream>

#include <var.h>

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
    // The .str() suppresses the double quotes by passing the raw char* to the
    // format operator.
    cout << "Using: " << s.str() << endl;
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
    if (arg.index("-f"))
        cout << "There's a -f" << endl;
    else
        cout << "There's no -f" << endl;

    // Given command line working, set a flag for valgrind
    bool vg = true;
    if (arg.index("-vg"))
        vg = false;

    // Basic numerical tests
    var s, w, x, y, z, dummy;
    cout << (s ? "true" : "false") << endl;
    cout << (!s ? "true" : "false") << endl;
    w = 'w';
    x = 2;
    y = 3.14;
    z = var::cos(y);
    s = "Hello!";
    cout << (s ? "true" : "false") << endl;
    cout << (!s ? "true" : "false") << endl;

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
    while (f.getline(is))
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
    xa[1] = xa[0];  // 1,1,3,4,5
    xa[2] += xa[1]; // 1,1,4,4,5
    xa[3] = 7;      // 1,1,4,7,5
    xa[4] += 7;     // 1,1,4,7,12
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

    // Modify non-trivial array value
    vmap["two"] += 10;
    cout << "modded vmap is " << vmap << endl;

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

    // Init from comma separated list
    var ai;
    ai = 5,4,3,2,1;
    cout << "ai is: " << ai << endl;

    // Read a text file via a dynamic library
    if (vg)
    {
        vfile txtf("txt");
        var txt = txtf.read("tests.txt");
        cout << "Loaded: " << txt << endl;
    }

    // Read a .ini file via a dynamic library
    if (vg)
    {
        vfile inif("ini");
        var ini = inif.read("tests.ini");
        cout << "Loaded: " << ini << endl;
    }

    // Plot something with gnuplot
    if (vg)
    {
        var gnu;
        gnu.push("plot sin(x), \"-\"");
        gnu.push(ai);
        vfile gnuf("gnuplot");
        gnuf.write("tests.eps", gnu);
    }

    // Tensors
    var ts = 0.0f;
    ts.resize(16);
    for (int i=0; i<16; i++)
        ts[i] = (float)i;
    var t1 = ts.view({4, 4});
    var t2 = ts.view({2, 2, 4});
    cout << ts << endl;
    cout << t1 << endl;
    cout << t2 << endl;
    cout << "t1(1,2): " << t1(1,2) << endl;
    cout << "t2(1,1,2): " << t2(1,1,2) << endl;
    t1(1,2) = 2.3f;
    cout << t1 << endl;

    var t3 = ts.view(t1.shape());
    cout << "Shape: " << t1.shape() << endl;
    cout << "New view: " << t3 << endl;

    // BLAS
    var bt;
    bt = 1.0f, 1.2f, 0.8f, -2.0f;
    if (vg)
    {
        cout << bt << "  sums to " << bt.sum()  << endl;
        cout << bt << " asums to " << bt.asum() << endl;
    }

    // Stream
    vstream vstr;
    float fl = 23e-1f;
    vstr << "H";
    cout << vstr.var() << endl;
    vstr << "ello: " << fl;
    cout << vstr.var() << endl;

    // gedcom
    if (vg)
    {
        vfile gedf("ged");
        var ged = gedf.read("tests.ged");
        cout << "Loaded: " << ged << endl;
    }

    // XML
    if (vg)
    {
        vfile xmlf("xml");
        var xml = xmlf.read("tests.xml");
        cout << "Loaded: " << xml << endl;
    }

    // wav
    if (vg)
    {
        vfile wavf("snd");
        var wav = wavf.read("tests.wav");
        cout << "Loaded wav file:" << endl;
        cout << " rate:     " << wav["rate"] << endl;
        cout << " frames:   " << wav["data"].shape(0) << endl;
        cout << " channels: " << wav["data"].shape(1) << endl;
    }

    // Exception
    try {
        throw vruntime_error(bt);
    }
    catch (vruntime_error e) {
        cout << "Caught: " << e.what() << endl;
    };

    // Init by overloading operator,()
    var comma;
    comma = 1.2, 2.0, 4, 5;
    cout << "Comma is: " << comma << endl;

    // Regular expressions
    cout << "Searching ello: " << ss << endl;
    if (ss.search("ello"))
        cout << "Matches" << endl;
    else
        cout << "Matches not" << endl;
    cout << "Matching \\S+ell\\S: " << ss << endl;
    if (ss.search("\\S+ell\\S"))
        cout << "Matches" << endl;
    else
        cout << "Matches not" << endl;
    var rep = ss.replace("lo", "ls bells");
    cout << "Replaced to: " << rep << endl;

    // Arrays with single initialiser
    var zeros(5, 0.0f);
    var ones(5, 1.0f);
    cout << "Zeros: " << zeros << endl;
    cout << "Ones:  " << ones << endl;

    // Broadcasting scalars
    t1 += 1;
    cout << "t1: " << t1 << endl;
    t1 *= 1.5;
    cout << "t1: " << t1 << endl;
    t1 += t1;
    cout << "t1: " << t1 << endl;
    t1 -= t1 - 1;
    cout << "t1: " << t1 << endl;

    // Because it works
    var cv = var::range('a', 'z').view({2,13});
    cout << "cv:\n" << cv << endl;

    // Sub-views
    var r12 = var::irange(0.1, 12.1);
    cout << "r12: " << r12 << endl;
    var sv = r12.view({4}, 4);
    cout << "sv: " << sv << endl;
    cout << "sv(0): " << sv(0) << endl;
    sv += 1;
    cout << "r12: " << r12 << endl;
    var rv;
    rv = -1.0, -2.0, -3.0, -4.0;
    cout << "rv: " << rv << endl;
    sv.offset(8);
    sv = rv;
    cout << "r12: " << r12 << endl;

    // Broadcasting tensors
    var r6 = var::irange(6.0).view({3,2});
    var r2;
    r2 = 1.0, 2.0;
    cout << "r2: " << r2 << endl;
    cout << "r6: " << r6 << endl;
    r6 += r2;
    cout << "r6: " << r6 << endl;
    r6 *= r2;
    cout << "r6: " << r6 << endl;
    cout << "r6*r6: " << r6 * r6 << endl;

    // View of a view
    var r3 = r6.view({2}, 0);
    var r4 = sv.view({2}, 0);
    cout << "r3: " << r3 << endl;
    cout << "r4: " << r4 << endl;

    // Functors
    Pow pow;
    Tan tan;
    cout << "Size: " << sizeof(Pow) << endl;
    cout << "Pow: " << pow(3.0, 2) << endl;
    cout << "Pow: " << pow(r6, 2) << endl;
    cout << "Tan: " << tan(r6) << endl;

    // Complex
    var fc = complex<float>(0.5, 0.7);
    cout << "fc: " << fc << endl;
    cout << "pow(fc,2): " << pow(fc, 2) << endl;
    cout << "tan(fc): " << tan(fc) << endl;
    var dc = complex<double>(0.2, 0.8);
    cout << "dc: " << dc << endl;
    cout << "pow(dc,2): " << pow(dc, 2) << endl;
    cout << "tan(dc): " << tan(dc) << endl;
    var dd = complex<double>(1.0, 1.2);
    dc += dd;
    cout << "dc: " << dc << endl;

    // DFT
    var td = view({2, 10});
    var fd;
    for (int i=0; i<10; i++)
    {
        td(0, i) = sinf(i);
        td(1, i) = cosf(i);
    }
    cout << "Time: " << td << endl;
    if (vg)
    {
        DFT dft(10);
        fd = dft(td);
        cout << "Freq: " << fd << endl;
    }

    var ab = var::abs(fd);
    cout << "Abs: " << ab << endl;

    // Phil, leave this at the end!
    return 0;
}
