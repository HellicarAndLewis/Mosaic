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

#ifndef GFX_BLUR_FBO_H
#define GFX_BLUR_FBO_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

#include <gfx/Blur.h>
#include <gfx/FBO.h>

namespace gfx {

  class BlurFBO {

  public:
    BlurFBO();
    ~BlurFBO();
    int init(int w, int h, double amount);
    void blur(GLuint tex);
    GLuint tex();                              /* returns the ID of the texture that contains the blurred image */
    
  public:
    int width;
    int height;
    int is_init;
    GLuint tex_pass0;
    GLuint tex_pass1;
    Blur blur_prog;
    FBO fbo;
    int win_width;
    int win_height;
  };

  inline GLuint BlurFBO::tex() {
    return tex_pass1;
  }

} /* namespace gfx */

#endif
