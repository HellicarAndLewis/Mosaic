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

  Blur
  ----
  A fast blur implementation

  Tip:
  ----
  Blur with a FBO that is half the size of the texture; this results in 
  more blur with lower values and less texture lookups.

  References: 
  -----------
  - https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
  - https://gist.github.com/roxlu/c1a3cb42a16c56cd68c2
  - http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

 */
#ifndef GFX_BLUR_H
#define GFX_BLUR_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

static const char* BLUR_VS = ""
  "#version 330\n"
  ""
  " const vec2[] pos = vec2[4]("
  "   vec2(-1.0, 1.0),"
  "   vec2(-1.0, -1.0),"
  "   vec2(1.0, 1.0),"
  "   vec2(1.0, -1.0)"
  "   );"
  ""
  "const vec2 texcoords[4] = vec2[] ("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  "); "
  ""
  "out vec2 v_tex;"
  ""
  "void main() {"
  "  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);"
  "  v_tex = texcoords[gl_VertexID];"
  " }" 
  "";


namespace gfx { 
  class Blur {
  public:
    Blur();
    ~Blur();
    int init(double amount);
    void blurX(float w, float h);
    void blurY(float w, float h);

  public:
    GLuint vao;
    GLuint vert;
    GLuint frag_y;
    GLuint frag_x;
    GLuint prog_x;
    GLuint prog_y;
    GLint xtex_w;
    GLint xtex_h;
    GLint ytex_w;
    GLint ytex_h;
  };
} /* namespace gfx */

#endif
