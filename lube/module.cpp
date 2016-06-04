/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2013
 */

#include <cassert>
#include <stdexcept>
#include <dlfcn.h>

#include "lube/module.h"


using namespace libube;


module::module(var iType)
{
    // Initialise
    mHandle = 0;
    char *dle = dlerror();

    // Open the library
    varstream lib;
    lib << "lib" << iType.str() << ".so";
    mHandle = dlopen(lib.str(), RTLD_LAZY);
    if ((dle = dlerror()) != NULL)
        throw error(dle);
    if (!mHandle)
        throw error("module::module(): dlopen failed");

    // Find the factory function
    *(void **)(&mFactory) = dlsym(mHandle, "factory");
    if ((dle = dlerror()) != NULL)
        throw error(dle);
}


module::~module()
{
    // Delete the instances before closing the dynamic library
    for (int i=0; i<mInstance.size(); i++)
        delete mInstance[i];
    dlclose(mHandle);
}


Module& module::create(var iArg)
{
    // I originally used a boost::ptr_vector for mInstance.  It seems too much
    // for this purpose though with it's "takes ownership" concept.  Here it's
    // the module that own the instance, and that can be handled fine using a
    // std::vector<Module*>.

    // Run the dynamically loaded factory function
    Module* inst = 0;
    (*mFactory)(&inst, iArg);
    if (!inst)
        throw error("Module factory failed");
    mInstance.push_back(inst);
    return *(mInstance.back());
}
