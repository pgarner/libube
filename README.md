# libube

## Introduction

`libube` could stand for Library for Union Broadcasting Extended, but it
doesn't.  Unless you're trying to sound professional then it's "lube".  Lube
greases the wheels of C++.
 
Of course, it's linked using `-lube`

## Variants

Lube defines a variant (lubricant!) called `var`.  When a `var` is set to A C++
native type it automatically behaves like that type.

    var w, x, y;
    w = 'w';
    x = 2;
    y = 3.14;

Native types include single and double precision complex numbers.

`var` is small enough to be passed by value.

## Arrays

When a `var` is an array then it is typically a dense array of its native type.
Arrays can be set using C++ initialiser lists.

    var a = {1.3f, 1.2, 45};

Comma separated values also work.  A `var` can also be an array of `var`.  An
array of type `char` is a string.

    var s = "Hello!";

perl-like string operations and regular expressions are implented for strings

    var n = "New string";
    var sp = n.split();

A view of an array can be defined using an array of `int`; this enables
vectors, matrices and arbitrary dimensional tensors

    var v1;
    v1 =
        0.0, 1.2, 1.4,
        1.0, 1.2, 1.4,
        2.0, 1.2, 1.4,
        3.0, 1.2, 1.4;
    v1 = v1.view({4,3});

Arrays can be queried, and assigned using the usual syntax, including
stack-like operations

    cout << a[0];
    a[1] = 4.6f;
    a.push(3.4f);

Arrays can be passed by value which actually updates a reference count.  They
are garbage collected by reference counting.  Return syntax is by value; it
uses C++11 move semantics.

## Operations

Many mathematical operators are defined on `var`, including most of the
standard mathematics library.

    z = y.cos();

Operators are implemented as functors.  When a `var` is an array, the operators
broadcast over the array as in matlab or numpy.

    v1 += 1.0f;
    var v2 = log(v1);

Arithmetic operations on arrays use BLAS; cmake's FindBLAS will use MKL if
available.  DFTs are a native operation.

`operator *` gives the Hadamard (element-wise) product.  For matrix
multiplication use `dot()`.

## Map types

A `var` can be a map type (associative array).  The map type can have arbitrary
depth, and can be mixed with array types as long as they are of type `var`.

    var vmap;
    vmap["one"] = 1;
    vmap["two"] = 2;
    vmap["three"] = 3;
    wmap["one"]["two"] = "three";
    wmap["one"][3]["four"] = "five";

The key need not be a string; it can be any `var`.

## Formatting

`operator <<` is defined for `var` such that a structure of any depth can be
formatted.  The format used is JSON, currently with extra syntax for `char`s
and complex numbers.

    cout << s[0] << endl;

## Modules

Lube is good at loading arbitrary file formats.  File loaders are modules that
are dynamically loaded.  When the parsing is done by an external library, the
dynamic library is linked to the external library meaning that the modules can
be disributed independently of lube and the external library.

Standard modules include `.ini` config files, `XML` via `expat`, text files,
and audio files via `sndfile`.

The module concept extends beyond file loading; there is a graph class that
wraps `boost::graph`.

--
[Phil Garner](http://www.idiap.ch/~pgarner)
September 2013