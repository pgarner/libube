/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2016
 */

#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include "lube/var.h"

namespace libube
{
    /**
     * Base class for modules (dynamically loaded classes)
     */
    class Module
    {
    public:
        virtual ~Module() {};
     };


    extern "C" {
        /**
         * Function with C linkage that must exist in the dynamically loaded
         * library.  It should return a module by calling new on the derived
         * class within the library.  It is part of the interface.
         */
        void factory(Module** oModule, var iArg);
    }


    /**
     * Module factory
     *
     * This is actually a loader for the module rather than the module itself.
     * The create() method is a factory method that generates modules.
     */
    class module
    {
    public:
        module(var iType);
        virtual ~module();
        virtual Module& create(var iArg=nil);
    protected:
        std::vector<Module*> mInstance; ///< Instances of module
    private:
        void (*mFactory)(Module** oModule, var iArg);
        void* mHandle;     ///< Handle for dynamic library
    };


    /**
     * Abstract class for dynamically loaded file handlers.  Defines the
     * interface that file handlers must implement.
     */
    class file : public Module
    {
    public:
        virtual ~file() {};
        virtual var read(var iFile) = 0;
        virtual void write(var iFile, var iVar) = 0;
    };


    /**
     * Class to dynamically load a file handler
     */
    class filemodule : public module
    {
    public:
        filemodule(var iType="txt") : module(iType) {}
        file& create(var iArg=nil) {
            return dynamic_cast<file&>(module::create(iArg));
        }
    };
}

#endif // MODULE_H
