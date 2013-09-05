# var: a C++ class

Perl and python are OK until you need to make a package.  Then it's so
much trouble you might as well have done it in C++.  libvar is about
making C++ easier.

var allows you to do this in C++:
```c++
var s, w, x, y, z, dummy;
w = 'w';
x = 2;
y = 3.14;
z = y.cos();
s = "Hello!";
cout << s[0] << endl;
```
i.e., it behaves in a similar way to variables in modern dynamic languages.

If a var is an array, it is reference to that array, meaning that it
can be passed around by value and returned from functions.  Arrays are
reference counted and hence garbage collected.

var is not a template; rather, the type is encoded within the class.
A var can change type.  The lack of templates means that compilation
is quite fast.

--
[Phil Garner](http://www.idiap.ch/~pgarner)
September 2013