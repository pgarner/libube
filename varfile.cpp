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

#include "var.h"


using namespace libvar;


module::module(const char* iType)
{
    // Initialise
    mHandle = 0;
    char *error = dlerror();

    // Open the library
    vstream lib;
    lib << "lib" << iType << ".so";
    mHandle = dlopen(lib.str(), RTLD_LAZY);
    if ((error = dlerror()) != NULL)
        throw std::runtime_error(error);
    if (!mHandle)
        throw std::runtime_error("module::module(): dlopen failed");

    // Find the factory function
    void (*factory)(Module** oModule);
    *(void **)(&factory) = dlsym(mHandle, "factory");
    if ((error = dlerror()) != NULL)
        throw std::runtime_error(error);

    // Run the dynamically loaded factory function
    (*factory)(&mInstance);
    if (!mInstance)
        throw std::runtime_error("module::module(): factory() failed");
}


module::~module()
{
    // Destroy the instance that the factory loaded
    if (mInstance)
        delete mInstance;

    // Close the dynamic library
    dlclose(mHandle);
}


Module* module::instance() const
{
    assert(mInstance);
    return mInstance;
}


vfile::vfile(const char* iType)
    : module(iType)
{
}


vfile::~vfile()
{
}
