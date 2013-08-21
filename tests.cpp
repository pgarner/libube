#include <cassert>
#include <var>
#include <map>
#include <fstream>

using namespace std;

var getString()
{
    var str;
    str = "An example string with spaces.";
    return str;
}

void useString(var s)
{
    cout << "Using: " << s << endl;
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
    var b = getString();
    cout << "b is: " << b << endl;
    useString(b);
    cout << "b is: " << b << endl;

    // Basic string split
    var sp;
    sp = b.split("n");
    cout << "sp is: " << sp << endl;

    // Basic string insert
    a.insert(1, "ddd");
    cout << "a is: " << a << endl;

    // Shifting of command line
    arg.insert(1, "insert");
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
    std::map<var, int> map;
    map["One"] = 1;
    map["Two"] = 2;
    map["Three"] = 3;
    cout << map.count("Zero") << endl;
    cout << map.count("One") << endl;

    // Read a small file
    var f;
    std::ifstream is("tests.txt", std::ifstream::in);
    if (is.fail())
        std::cout << "Open failed" << std::endl;
    while (f.getline(is).defined())
        cout << f << endl;

    // Shallow copy
    var c1 = arg;
    c1.push("extra");
    cout << c1 << endl;
    cout << arg << endl;
    var c2 = arg.copy();
    c2.pop();
    cout << c2 << endl;
    cout << arg << endl;

    return 0;
}
