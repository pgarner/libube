#include <cassert>
#include <var>

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
    var arg;
    for (int i=0; i<argc; i++)
        arg.push(argv[i]);
    cout << "Arg: " << arg << endl;
    cout << "Arg[0]: " << arg[0] << endl;
    cout << "Arg[0][0]: " << arg[0][0] << endl;

    if (arg[0][0] == '.')
        cout << "It's a dot" << endl;
    else
        cout << "It's not a dot" << endl;

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

    return 0;
}
