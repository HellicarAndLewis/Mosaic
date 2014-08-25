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
#ifndef VIDEO_YUV420P_H
#define VIDEO_YUV420P_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#include <tinylib.h>

static const char* YUV420P_VS = "" 
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
  "  vec2(0.0, 0.0), "
  "  vec2(0.0, 1.0), "
  "  vec2(1.0, 0.0), "
  "  vec2(1.0, 1.0)  "
  "); "
  ""
  "out vec2 v_coord;"
  ""
  "void main() {"
  "  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);"
  "  v_coord = texcoords[gl_VertexID];"
  " }" 
  "";
 
static const char* YUV420P_FS = ""
  "#version 330\n"
  "uniform sampler2D y_tex;"
  "uniform sampler2D u_tex;"
  "uniform sampler2D v_tex;"
  "in vec2 v_coord;"
  "layout( location = 0 ) out vec4 fragcolor;"
  ""
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  ""
  "void main() {"
  "  float y = texture(y_tex, v_coord).r;"
  "  float u = texture(u_tex, v_coord).r;"
  "  float v = texture(v_tex, v_coord).r;"
  "  vec3 yuv = vec3(y,u,v);"
  "  yuv += offset;"
  "  fragcolor = vec4(0.0, 0.0, 0.0, 1.0);"
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
  "}"
  "";
 
namespace vid {

  class YUV420P {

  public:
    YUV420P();
    ~YUV420P();
    int init(int w, int h);                                                                    /* initialize the YUV420 shader / textures */
    int update(uint8_t* y, int ystride, uint8_t* u, int ustride, uint8_t* v, int vstride);     /* update the textures */
    void draw();
    int shutdown();                                                                            /* frees allocated textures/progs/etc.. */

  public:
    GLuint vao;
    GLuint prog;
    GLuint frag;
    GLuint vert;
    GLuint tex_y;
    GLuint tex_u;
    GLuint tex_v;
    int w;
    int h;
    int hh;
    int hw;
  };

} /* namespace vid */

#endif
