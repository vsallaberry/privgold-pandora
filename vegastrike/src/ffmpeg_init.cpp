//
// C++ Implementation: vid_file
//

#include "vsfilesystem.h"
#include "config.h"
#include "ffmpeg_init.h"

#include <string.h>
#include <utility>

// define a 128k buffer for video streamers
#define BUFFER_SIZE 128*(1<<10)

#ifndef ENOENT
#define ENOENT 2
#endif


/* FOLLOWING CODE IS ONLY INCLUDED IF YOU HAVE FFMPEG */
/* ******************************************** */
#ifdef HAVE_FFMPEG
#if defined(_WIN32) && defined(HAVE_XOFFSET_T)
#define offset_t xoffset_t
#endif

#ifndef offset_t
    #if (LIBAVCODEC_VERSION_MAJOR >= 52) || (LIBAVCODEC_VERSION_INT >= ((51<<16)+(49<<8)+0))  ||   defined(__amd64__) || defined(_M_AMD64)   ||   defined(__x86_64) || defined(__x86_64__) || defined(__arm64__)
        typedef int64_t offset_t;
    #else
        typedef int offset_t;
    #endif
#endif

using namespace VSFileSystem;

extern "C" int _url_open(URLContext *h, const char *filename, int flags)
{
    if (strncmp(filename,"vsfile:",7)!=0)
        return AVERROR(ENOENT);

    const char* type = strchr(filename+7, '|');
    std::string path(filename+7, type?type-filename-7:strlen(filename+7));
    VSFileType vstype = ( type ? (VSFileType)atoi(type) : VideoFile );

    VSFile *f = new VSFile();
    if (f->OpenReadOnly(path, vstype) > Ok) {
        delete f;
        return AVERROR(ENOENT);
    } else {
        h->priv_data = f;
        return 0;
    }
}

extern "C" int _url_close(URLContext *h)
{
    delete (VSFile*)(h->priv_data);
    return 0;
}

extern "C" int _url_read(URLContext *h, unsigned char *buf, int size)
{
    return ((VSFile*)(h->priv_data))->Read(static_cast<void*>(buf), size);
}

extern "C" int _url_write(URLContext *h, const unsigned char *buf, int size)
{
    // read-only please
    return 0;
}

extern "C" offset_t _url_seek(URLContext *h, offset_t pos, int whence)
{
    if (whence != AVSEEK_SIZE) {
        ((VSFile*)(h->priv_data))->GoTo(long(pos));
        return ((VSFile*)(h->priv_data))->GetPosition();
    } else {
        return ((VSFile*)(h->priv_data))->Size();
    }
}


struct URLProtocol vsFileProtocol = {
    "vsfile",
    _url_open,
    _url_read,
    _url_write,
    _url_seek,
    _url_close,
};

namespace FFMpeg {

    void initLibraries()
    {
        static bool initted = false;
        if (!initted) {
            initted = true;
            av_register_all();
            register_protocol(&vsFileProtocol);
        }
    }

};

#else // No FFMPEG

namespace FFMpeg {

    void initLibraries()
    {
        // No-op stub
    };

};

#endif

