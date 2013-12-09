/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2013
 */

#include <var>
#include <sndfile.h>

#include "varfile.h"

void read(const char* iFile, var& oVar)
{
    SF_INFO sfInfo;
    sfInfo.format = 0;
    SNDFILE* sndFile = sf_open(iFile, SFM_READ, &sfInfo);
    if (!sndFile)
        throw vruntime_error("sndfile::read: Failed to open file");

    oVar.clear();
    oVar["rate"] = sfInfo.samplerate;
    oVar["channels"] = sfInfo.channels;
    int size = sfInfo.frames * sfInfo.channels;
    var data = 0.0f;
    data.resize(size);
    int nGot = sf_readf_float(sndFile, (float*)&(data), size);
    if (nGot != size)
        throw vruntime_error("sndfile::read: Size fuckup");
    oVar["data"] = data;
}

void write(const char* iFile, var iVar)
{
}
