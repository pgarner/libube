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
    var lib = "lib";
    lib += iType;
    lib += ".so";
    void* handle = dlopen(&lib, RTLD_LAZY);
    if (!handle)
        throw std::runtime_error("var::read(): dlopen failed");

    // Find the function
    char *error;
    var (*dynamicRead)(const char* iFile);
    dlerror();
    *(void **)(&dynamicRead) = dlsym(handle, "read");
    if ((error = dlerror()) != NULL)
        throw std::runtime_error(error);

    // Run the dynamically loaded function
    *this = (*dynamicRead)(iFile);

    // Close the library
    dlclose(handle);
    return *this;
}


var& var::write(const char* iFile, const char* iType)
{
    // Open the library
    var lib = "lib";
    lib += iType;
    lib += ".so";
    void* handle = dlopen(&lib, RTLD_LAZY);
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
