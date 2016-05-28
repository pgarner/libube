/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2016
 */

#include <curl/curl.h>
#include <lube/curl.h>
#include <lube.h>

namespace libube
{
    /**
     * The name of the library is actually cURL; it's a sort of reverse
     * camelCase.  Not even the curl library itself used this form.  Here, we
     * need to avoid the name CURL as it's used in the library.  It's purely
     * local to this file, so, fuck it: Curly.
     */
    class Curly : public curl
    {
    public:
        Curly();
        ~Curly();
        virtual var transfer(var iURL);
    };

    void factory(Module** oModule, var iArg)
    {
        *oModule = new Curly;
    }

    /**
     * A C++ wrapper for the curl easy interface.
     * ...not that it's really necessary, but hey.
     */
    class Easy
    {
    public:
        Easy(var iURL);
        ~Easy();
        var perform();
        size_t writeCallback(char* iPtr, size_t iSize, size_t iNMemb);
    private:
        CURL* mHandle;
        var mData;
    };
}

using namespace libube;

Curly::Curly()
{
    // It's possible that this should be module-wide
    curl_global_init(CURL_GLOBAL_ALL);
}

Curly::~Curly()
{
    curl_global_cleanup();
}

var Curly::transfer(var iURL)
{
    Easy easy(iURL);
    var stuff = easy.perform();
    return stuff;
}

/**
 * The actual callback.  Passes the call on to the member function.
 */
static size_t WriteCallback(
    char* iPtr, size_t iSize, size_t iNMemb, void* iUserData
)
{
    return ((Easy*)iUserData)->writeCallback(iPtr, iSize, iNMemb);    
}

Easy::Easy(var iURL)
{
    // Initialise an easy handle
    mData = "";
    mHandle = curl_easy_init();
    if (!mHandle)
        throw error("curl_easy_init() failed");

    // Set the URL to read and a callback mechanism for the actual data
    curl_easy_setopt(mHandle, CURLOPT_URL, iURL.str());
    curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, this);
}

Easy::~Easy()
{
    // Close the easy handle
    curl_easy_cleanup(mHandle);
    mHandle = 0;
}

var Easy::perform()
{
    CURLcode res = curl_easy_perform(mHandle);
    if (res != CURLE_OK)
    {
        var err = curl_easy_strerror(res);
        throw error(err);
    }
    return mData;
}

size_t Easy::writeCallback(char* iPtr, size_t iSize, size_t iNMemb)
{
    // This actually involves two copies, one to str and one to mData.
    // Optimisation is a problem for future me.
    size_t s = iSize * iNMemb;
    var str(s, iPtr);
    mData.append(str);
    return s;
}
