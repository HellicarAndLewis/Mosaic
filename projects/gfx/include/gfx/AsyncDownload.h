#ifndef ASYNC_DOWNLOAD_H
#define ASYNC_DOWNLOAD_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

namespace gfx {

  class AsyncDownload {
  public:
    AsyncDownload();
    ~AsyncDownload();
    int init(int w, int h, GLenum fmt);
    int download(); /* assumes the current read buffer has been set by the called */

  public:
    GLuint pbo[2];
    GLenum format;
    GLenum read_format;
    int dx;
    int width;
    int height;
    int nbytes;  /* number of bytes in the buffer into which we download */
    unsigned char* buffer;
  };

} /* namespace gfx */

#endif
