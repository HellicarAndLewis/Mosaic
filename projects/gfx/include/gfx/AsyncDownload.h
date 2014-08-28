/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/
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
    int init(int w, int h, GLenum fmt);   /* allocate memory; creates GL objects */
    int shutdown();                       /* frees all memory; destroys GL objects */
    int download();                       /* assumes the current read buffer has been set by the called */

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
