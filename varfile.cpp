/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2013
 */

#include <var>
#include <stdexcept>
#include <dlfcn.h>


var& var::read(const char* iFile, const char* iType)
{
    // Open the library
    void* handle = dlopen("libtxt.so", RTLD_LAZY);
    if (!handle)
        throw std::runtime_error("var::read(): dlopen failed");

    // Find the function
    char *error;
    void (*dynamicRead)(const char* iFile, var& iVar);
    dlerror();
    *(void **)(&dynamicRead) = dlsym(handle, "read");
    if ((error = dlerror()) != NULL)
        throw std::runtime_error(error);

    // Run the dynamically loaded function
    (*dynamicRead)(iFile, *this);

    // Close the library
    dlclose(handle);
    return *this;
}


var& var::write(const char* iFile, const char* iType)
{
    // Open the library
    void* handle = dlopen("libtxt.so", RTLD_LAZY);
    if (!handle)
        throw std::runtime_error("var::read(): dlopen failed");

    // Find the function
    char *error;
    void (*dynamicWrite)(const char* iFile, var iVar);
    dlerror();
    *(void **)(&dynamicWrite) = dlsym(handle, "write");
    if ((error = dlerror()) != NULL)
        throw std::runtime_error(error);

    // Run the dynamically loaded function
    (*dynamicWrite)(iFile, *this);

    // Close the library
    dlclose(handle);
    return *this;
}
