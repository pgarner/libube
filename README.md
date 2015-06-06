# libube: Library for Union Broadcasting Extended

## Overview

`libube` could stand for Library for Union Broadcasting Extended.  It is is a
collection of several loosely linked ideas stemming from experience programming
both in C++ and in dynamic languages such as perl, python and ruby.  At its
heart is a C++ class, `var`, which behaves more like a variable in a dynamic
language.  `var` is not a template; rather, the type is encoded within the
class.

Of course, it's linked using `-lube`; lube greases the wheels of C++.

`libube` also contains mechanisms for dynamically loading modules and for
broadcasting operations over arrays.

Part of the rationale of `libube` is that perl and python are very convenient
for writing certain types of program (ad-hoc parsers, text processors; anything
with associative arrays).  However, they become inconvenient as such programs
become large.  Of course, 'large' is subjective (as is 'inconvenient'), but one
example situation is when you need to make a package (split the source over
multiple files).  Another situation is when the program must be optimised such
that part of it has to be written in C or C++.  These situations are normal in
C++, all the build systems handle the resulting headers and libraries easily.
The question arises: Can C++ be made as convenient as the dynamic languages,
whist retaining the infrastructure for program growth?  That is, if you know a
program is going to grow, can you start in C++ in the first place?  `libube` is
an experiment; it tries to answer that question.

## Examples

var allows you to do things like this in C++:

    var s, w, x, y, z;
    w = 'w';
    x = 2;
    y = 3.14;
    z = y.cos();
    s = "Hello!";
    cout << s[0] << endl;

i.e.,
* You don't need to specify the type.
* Strings are handled implicitly.
* Common functions that depend on type are handled by the library.

## Arrays

The natural array type in python is a polymorphic list; that is, it
can hold distinct types and supports insertion and deletion quickly.
By contrast, the natural array in `libube` is a dense array of a
builtin type (or an array of `var`).  This means that an array of type
`int` can only hold type `int`.  A libube string is such an array of
type `char`.  It can act efficently as an array or stack; insertion
and deletion are less efficient.  It is analogous to a numpy
`ndarray`, also supporting views, hence tensors.

If a var is an array, it is a reference to that array, meaning that it
can be passed around by value and returned from functions.  Arrays are
reference counted and hence garbage collected.


--
[Phil Garner](http://www.idiap.ch/~pgarner)
September 2013