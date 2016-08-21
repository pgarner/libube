/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#ifndef FUNC_H
#define FUNC_H

namespace libube
{
    // Predeclarations
    class var;
    class ind;


    /**
     * Base functor
     */
    class Functor
    {
    public:
        Functor() { mDim = 0; };
        virtual ~Functor() {};
    protected:
        int mDim;
    };


    /**
     * Unary functor
     *
     * A unary functor just acts on itself.  mDim indicates the dimension of
     * the operation.
     */
    class UnaryFunctor : public Functor
    {
    public:
        var operator ()(const var& iVar) const;
        var operator ()(const var& iVar, var& oVar) const;
    protected:
        virtual var alloc(var iVar) const;
        virtual void broadcast(var iVar, var& oVar) const;
        virtual void scalar(const var& iVar, var& oVar) const;
        virtual void vector(
            var iVar, ind iOffsetI, var& oVar, ind iOffsetO
        ) const;
        virtual void vector(var iVar, var& oVar) const;
    };

    /**
     * Binary functor
     *
     * A binary functor broadcasts across two inputs together.
     */
    class BinaryFunctor : public Functor
    {
    public:
        var operator ()(
            const var& iVar1, const var& iVar2
        ) const;
        var operator ()(
            const var& iVar1, const var& iVar2, var& oVar
        ) const;
    protected:
        virtual var alloc(var iVar1, var iVar2) const;
        virtual void broadcast(var iVar1, var iVar2, var& oVar) const;
        virtual void scalar(
            const var& iVar1, const var& iVar2, var& oVar
        ) const;
        virtual void vector(
            var iVar1, ind iOffset1,
            var iVar2, ind iOffset2,
            var& oVar, ind iOffsetO
        ) const;
        virtual void vector(var iVar1, var iVar2, var& oVar) const;
    };


    /**
     * Arithmetic functor
     *
     * An arithmetic functor is a binary functor, a functor of two variables,
     * that broadcasts in an arithmetic sense, i.e., var2 is repeated as a
     * whole across var2 sized bits of var1.
     */
    class ArithmeticFunctor : public BinaryFunctor
    {
    protected:
        virtual void broadcast(var iVar1, var iVar2, var& oVar) const;
    };


    /**
     * N-ary functor
     *
     * An N-ary functor has N arguments.  It broadcasts over the common
     * dimension of each.
     */
    class NaryFunctor : public Functor
    {
    public:
        var operator ()() const;
        var operator ()(const var& iVar) const;
        var operator ()(const var& iVar, var& oVar) const;
    protected:
        virtual var alloc(var iVar) const;
        virtual void broadcast(var iVar, var& oVar) const;
        virtual void scalar(const var& iVar, var& oVar) const;
        virtual void vector(var iVar, var& oVar) const;
    };
}

#endif // FUNC_H
