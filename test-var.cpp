#include <cassert>
#include <map>

#include "lube.h"

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
    // Timer; commented as it will never match the reference output
    //lube::timer d("Program duration");

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

    // Basic numerical tests
    var s, w, x, y, z, dummy;
    cout << (s ? "true" : "false") << endl;
    cout << (!s ? "true" : "false") << endl;
    w = 'w';
    x = 2;
    y = 3.14;
    z = lube::cos(y);
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
    cout << -y << endl;

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

    // Split with empty strings
    var vcn = "N:First;Last;;;";
    cout << vcn << " splits to " << vcn.split(";") << endl;

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

    // .str() should give the entry without quotes
    cout << "wmap entry is " << wmap["one"]["two"] << endl;
    cout << "wmap entry is " << wmap["one"]["two"].str() << endl;

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

    // Exception
    try {
        throw lube::error(ts);
    }
    catch (lube::error e) {
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
    if (ss.match("\\S+ell\\S"))
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

    // Because it works
    var cv = lube::range('a', 'z').view({2,13});
    cout << "cv:\n" << cv << endl;

    // Complex
    var fc = complex<float>(0.5, 0.7);
    cout << "fc: " << fc << endl;
    cout << "pow(fc,2): " << lube::pow(fc, 2) << endl;
    cout << "tan(fc): " << lube::tan(fc) << endl;
    var dc = complex<double>(0.2, 0.8);
    cout << "dc: " << dc << endl;
    cout << "pow(dc,2): " << lube::pow(dc, 2) << endl;
    cout << "tan(dc): " << lube::tan(dc) << endl;
    var dd = complex<double>(1.0, 1.2);
    dc += dd;
    cout << "dc: " << dc << endl;

    // Case change
    var mix = "This was mixed cäsÉ";
    cout << "Upper: " << mix.copy().toupper() << endl;
    cout << "Lower: " << mix.copy().tolower() << endl;

    // Broadcast over strings
    var strs;
    strs.push("One two three o'clock four o'clock rock");
    strs.push("Five six seven o'clock eight o'clock rock");
    var STRS = lube::toupper(strs);
    cout << "Lower: " << strs.tolower() << endl;
    cout << "Upper: " << STRS << endl;

    // Initialise an empty pair
    var nilPair;
    nilPair[lube::nil];
    cout << "Nil pair: " << nilPair << endl;
    nilPair["Hi"] = 3;
    cout << "Nil pair: " << nilPair << endl;
    nilPair[lube::nil] = 4;
    cout << "Nil pair: " << nilPair << endl;

    // UTF-8
    var utf;
    utf["jp"] = "日本語です";
    utf["fr"] = "C'est Français";
    cout << utf << endl;

    // Concat string
    var concat;
    concat = "one", " ", "two";
    cout << concat << endl;

    // Initialiser list
    var init({strs, mix});
    cout << "init: " << init << endl;

    // Phil, leave this at the end!
    return 0;
}
