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
        virtual ~Functor() {};
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
        UnaryFunctor() { mDim = 0; };
        var operator ()(const var& iVar) const;
        var operator ()(const var& iVar, var& oVar) const;
    protected:
        int mDim;
        virtual var alloc(var iVar) const;
        virtual void broadcast(var iVar, var& oVar) const;
        virtual void scalar(const var& iVar, var& oVar) const;
        virtual void vector(
            var iVar, ind iOffsetI, var& oVar, ind iOffsetO
        ) const;
        virtual void vector(var iVar, var& oVar) const;
    };

#   define BASIC_UNARY_FUNCTOR_DECL(f)                      \
    class f : public UnaryFunctor                           \
    {                                                       \
    protected:                                              \
        void scalar(const var& iVar, var& oVar) const;      \
    };
}

#endif // FUNC_H
