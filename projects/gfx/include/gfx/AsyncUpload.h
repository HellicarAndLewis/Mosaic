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

  AsyncUpload
  ------------
  
  This class uses a couple of pixel buffer objects to overcome
  synchronization issues when uploading data to the gpu.  It's build for 
  a specific project where we had to upload the complete pixel buffer.
  
  
  Make sure that you use GL_BGRA as format with GL_UNSIGNED_INT_8_8_8_8_REV
  as type for the texture that you're using (on mac). When you use only 3 
  channel images, you won't get the optimal upload path and performance is 
  extremelly reduced. 

  Use something like:
  --
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
  --

*/
#ifndef ASYNC_UPLOAD_H
#define ASYNC_UPLOAD_H

#include <stdint.h>
#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#define ASYNC_UPLOAD_NUM_BUFFERS 3

namespace gfx {

  class AsyncUpload {
  public:
    AsyncUpload();
    ~AsyncUpload();
    int init(int w, int h, GLenum fmt);         /* allocates memory; creates GL objects. */
    int shutdown();                             /* frees all memory; destroys GL objects. */
    int upload(unsigned char* pixels);          /* upload the given pixels, we assume that you have bound a texture of the same dimensions as our buffers */

  public:
    GLuint pbo[ASYNC_UPLOAD_NUM_BUFFERS];      /* the pbos that are created in init(), and removed in shutdown(). */ 
    GLenum format;                             /* what format is used */ 
    int dx;                                    /* current index into the pbo array */
    int width;                                 /* width of the texture */
    int height;                                /* height of the texture */
    int nbytes;                                /* number of bytes in the PBOs */
    int channels;                              /* number of color channels that are used */
    uint64_t n;                                /* number of uploads, used to 'schedule' what PBO we should use. */
  };

} /* namespace gfx */

#endif
