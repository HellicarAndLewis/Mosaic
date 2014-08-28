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
#ifndef GFX_IMAGE_H
#define GFX_IMAGE_H

#include <string>
#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#define IMAGE_NONE 0
#define IMAGE_JPEG 1
#define IMAGE_PNG 2

namespace gfx {
  
  class Image {

  public:
    Image();
    ~Image();
    int load(std::string filepath);
    int createTexture();
    int updateTexture();
    
  public:
    int type;
    int width;
    int height;
    int channels;
    int capacity;  /* how much bytes we can store in Image::pixels */
    unsigned char* pixels;
    GLuint texid;
  };

} /* namespace gfx */

#endif
