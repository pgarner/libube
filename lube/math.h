/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#ifndef MATH_H
#define MATH_H

#include <complex>
#include <lube/func.h>

namespace libube
{
    // Predeclarations
    class ind;
    class var;

    // The complex types
    typedef std::complex<float> cfloat;
    typedef std::complex<double> cdouble;

    /**
     * Functor for things like imag() and abs() where the argument is complex
     * but the return type is always real.
     */
    class RealUnaryFunctor : public UnaryFunctor
    {
    protected:
        virtual var alloc(var iVar) const;
    };

#   define BASIC_UNARY_FUNCTOR_DECL(f)                      \
    class f : public UnaryFunctor                           \
    {                                                       \
    protected:                                              \
        void scalar(const var& iVar, var& oVar) const;      \
    };

#   define REAL_UNARY_FUNCTOR_DECL(f)                       \
    class f : public RealUnaryFunctor                       \
    {                                                       \
    protected:                                              \
        void scalar(const var& iVar, var& oVar) const;      \
    };

#   define BASIC_ARITH_FUNCTOR_DECL(f)                      \
    class f : public ArithmeticFunctor                      \
    {                                                       \
    protected:                                              \
        void scalar(                                        \
            const var& iVar1, const var& iVar2, var& oVar   \
        ) const;                                            \
    };

    // Math functors
    BASIC_UNARY_FUNCTOR_DECL(Sin)
    BASIC_UNARY_FUNCTOR_DECL(Cos)
    BASIC_UNARY_FUNCTOR_DECL(Tan)
    BASIC_UNARY_FUNCTOR_DECL(ATan)
    BASIC_UNARY_FUNCTOR_DECL(Floor)
    BASIC_UNARY_FUNCTOR_DECL(Sqrt)
    BASIC_UNARY_FUNCTOR_DECL(Log)
    BASIC_UNARY_FUNCTOR_DECL(Exp)
    BASIC_ARITH_FUNCTOR_DECL(Pow)
    REAL_UNARY_FUNCTOR_DECL(Real)
    REAL_UNARY_FUNCTOR_DECL(Imag)
    REAL_UNARY_FUNCTOR_DECL(Abs)
    REAL_UNARY_FUNCTOR_DECL(Arg)
    REAL_UNARY_FUNCTOR_DECL(Norm)

    /**
     * Set/Copy functor
     */
    class Set : public ArithmeticFunctor
    {
    protected:
        void scalar(const var& iVar1, const var& iVar2, var& oVar) const;
        void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
    };


    /**
     * Addition functor
     */
    class Add : public ArithmeticFunctor
    {
    protected:
        void scalar(const var& iVar1, const var& iVar2, var& oVar) const;
        void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
    };


    /**
     * Subtraction functor
     */
    class Sub : public ArithmeticFunctor
    {
    protected:
        void scalar(const var& iVar1, const var& iVar2, var& oVar) const;
        void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
    };


    /**
     * Multiplication functor
     */
    class Mul : public ArithmeticFunctor
    {
    protected:
        void broadcast(var iVar1, var iVar2, var& oVar) const;
        void scalar(const var& iVar1, const var& iVar2, var& oVar) const;
        void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
        void scale(var iVar1, var iVar2, var& oVar, int iOffset) const;
    };


    /**
     * Dot product functor
     */
    class Dot : public ArithmeticFunctor
    {
    protected:
        var alloc(var iVar1, var iVar2) const;
        void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
    };


    /**
     * Division functor
     * There's no array operation.
     */
    class Div : public ArithmeticFunctor
    {
    protected:
        void scalar(const var& iVar1, const var& iVar2, var& oVar) const;
    };


    /**
     * Absolute sum functor
     */
    class ASum : public UnaryFunctor
    {
    public:
        ASum() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        void scalar(const var& iVar, var& oVar) const;
        void vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const;
    };


    /**
     * Basic sum functor
     */
    class Sum : public UnaryFunctor
    {
    public:
        Sum() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        void scalar(const var& iVar, var& oVar) const;
        void vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const;
    };


    /**
     * Transpose functor
     */
    class Transpose : public UnaryFunctor
    {
    public:
        Transpose() { mDim = 2; };
    protected:
        var alloc(var iVar) const;
        void scalar(const var& iVar, var& oVar) const;
        void vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const;
    };


    /**
     * Index of absolute maximum value functor
     */
    class IAMax : public UnaryFunctor
    {
    public:
        IAMax() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        void vector(
            var iVar, ind iOffsetI, var& oVar, ind iOffsetO
        ) const;
    };


    /**
     * Polynomial roots functor
     */
    class Roots : public UnaryFunctor
    {
    public:
        Roots() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        void vector(var iVar, var& oVar) const;
    };


    /**
     * Polynomial functor
     */
    class Poly : public UnaryFunctor
    {
    public:
        Poly() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        void vector(var iVar, var& oVar) const;
    };


    /**
     * Sorting functor
     */
    class Sort : public UnaryFunctor
    {
    public:
        Sort() { mDim = 1; };
    protected:
        void vector(var iVar, var& oVar) const;
    };


    /**
     * Concatenating functor
     */
    class Concatenate : public NaryFunctor
    {
    public:
        Concatenate() { mDim = 1; };
    protected:
        var alloc(var iVar) const;
        virtual void vector(var iVar, var& oVar) const;
    };


    // stdlib Functors
    extern Sin sin;
    extern Cos cos;
    extern Tan tan;
    extern ATan atan;
    extern Floor floor;
    extern Sqrt sqrt;
    extern Log log;
    extern Exp exp;
    extern Pow pow;
    extern Real real;
    extern Imag imag;
    extern Abs abs;
    extern Arg arg;
    extern Norm norm;

    // BLAS functors
    extern Set set;
    extern Add add;
    extern Sub sub;
    extern Mul mul;
    extern Dot dot;
    extern Div div;
    extern ASum asum;
    extern Sum sum;
    extern IAMax iamax;

    // BLAS-like
    extern Transpose transpose;
    extern Concatenate concatenate;

    // Lapack based functors
    extern Roots roots;
    extern Poly poly;

    // stdlib
    extern Sort sort;
}

#endif // MATH_H
