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
    var arg(argc, argv);
    cout << "Arg: " << arg << endl;
    cout << "Arg[0]: " << arg[0] << endl;
    cout << "Arg[0][0]: " << arg[0][0] << endl;

    if (arg[0][0] == '.')
        cout << "It's a dot" << endl;
    else
        cout << "It's not a dot" << endl;

    if (arg.index("-f").defined())
        cout << "There's a -f" << endl;
    else
        cout << "There's no -f" << endl;

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

    x += 2;
    y += 3.14f;
    x.push(2);
    y -= 1;

    cout << x << endl;
    cout << y << endl;

    var a = s;
    cout << "a is: " << a << endl;
    a = "New string";
    
    var n = "New string";
    if (a == n)
        cout << "equal" << endl;
    else
        cout << "not equal" << endl;

    var b = getString();
    cout << "b is: " << b << endl;
    useString(b);
    cout << "b is: " << b << endl;

    var sp;
    sp = b.split("n");
    cout << "sp is: " << sp << endl;

    a.insert(1, "ddd");
    cout << "a is: " << a << endl;
    arg.insert(1, "insert");
    cout << "arg is: " << arg << endl;
    arg.remove(0);
    cout << "arg is: " << arg << endl;
    var as = arg.shift();
    cout << "arg is: " << arg << " shifted: " << as << endl;
    cout << "Joining: " << arg << endl;
    cout << "Joined: " << arg.join("-") << endl;

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

    return 0;
}
