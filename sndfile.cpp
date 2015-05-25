/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2013
 */

#include <sndfile.h>
#include <var.h>

namespace libvar
{
    class sndfile : public File
    {
    public:
        sndfile(var iAttr) { mAttr = iAttr; };
        virtual var read(var iFile);
        virtual void write(var iFile, var iVar);
    private:
        var mAttr;
    };


    void factory(Module** oModule, var iArg)
    {
        *oModule = new sndfile(iArg);
    }
}


using namespace libvar;


/*
 * http://www.mega-nerd.com/libsndfile/api.html
 */

var sndfile::read(var iFile)
{
    // Open the file
    SF_INFO sfInfo;
    sfInfo.format = 0;
    SNDFILE* snd = sf_open(iFile.str(), SFM_READ, &sfInfo);
    if (!snd)
        throw error("sndfile::read: Failed to open file");

    // Read stuff
    mAttr["rate"] = sfInfo.samplerate;
    int size = sfInfo.frames * sfInfo.channels;
    var data = 0.0f;
    data.resize(size);
    int nGot = sf_readf_float(snd, data.ptr<float>(), size);
    if (nGot != size)
        throw error("sndfile::read: Size fuckup");
    if (sfInfo.channels != 1)
        // In place transpose for channels > 1
        data = data.view({(int)sfInfo.frames, sfInfo.channels}).transpose();
    sf_close(snd);
    return data;
}

void sndfile::write(var iFile, var iVar)
{
    // Open the file
    SF_INFO sfInfo;
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_ENDIAN_FILE;
    sfInfo.samplerate = mAttr["rate"].get<int>();
    sfInfo.channels = iVar.dim() > 1 ? iVar.shape(0) : 1;
    SNDFILE* snd = sf_open(iFile.str(), SFM_WRITE, &sfInfo);
    if (!snd)
        throw error("sndfile::write: Failed to open file");

    // Write stuff
    int n;
    if (iVar.dim() > 1)
    {
        var tr = transpose(iVar);
        n = (int)sf_writef_float(snd, tr.ptr<float>(), tr.size());
    }
    else
        n = (int)sf_writef_float(snd, iVar.ptr<float>(), iVar.size());
    sf_close(snd);
    if (iVar.size() != n)
        throw error("sndfile::write: Size fuckup");
}
