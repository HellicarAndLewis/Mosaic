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
#ifndef ROXLU_AVERAGE_COLOR_H
#define ROXLU_AVERAGE_COLOR_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_OPENGL
#define ROXLU_USE_MATH
#define ROXLU_USE_FONT
#include <tinylib.h>

#include <gfx/AsyncDownload.h>

static const char* AVG_COL_VS = ""
  "#version 330\n"
  ""
  " const vec2[] pos = vec2[4]("
  "   vec2(-1.0, 1.0),"
  "   vec2(-1.0, -1.0),"
  "   vec2(1.0, 1.0),"
  "   vec2(1.0, -1.0)"
  "   );"
  ""
  "void main() {"
  "  gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);"
  " }" 
  "";

static const char* AVG_COL_FS = ""
  "#version 330\n"
  "uniform sampler2D u_tex;"
  "uniform int u_tile_size;"
  "uniform int u_cols; " 
  "uniform int u_rows;"
  "uniform int u_image_w; " 
  "uniform int u_image_h; "
  "out vec4 fragcolor;"
  "void main() {"
  "  fragcolor = vec4(0.0, 0.0, 0.0, 1.0);"
  "  vec2 texcoord = vec2(gl_FragCoord.x * (1.0/u_cols) , gl_FragCoord.y * (1.0/u_rows));"
  "  vec4 texcol = texture(u_tex, texcoord);"
  "  int start_x = int(gl_FragCoord.x * u_tile_size);"
  "  int start_y = int(gl_FragCoord.y * u_tile_size);"
  "  int i, j;"
  "  for (i = start_x ; i < (start_x + u_tile_size); ++i) {"
  "    for (j = start_y; j < (start_y + u_tile_size); ++j) {"
  "      texcoord = vec2(i * (1.0/u_image_w), j * (1.0/u_image_h));"
  "      texcol = texture(u_tex, texcoord);"
  "      fragcolor += texcol;" 
  "    }"
  "  }"
  " fragcolor /= (u_tile_size * u_tile_size ); "
  //  "  fragcolor.rgb = texcol.rgb;"
  "}"
  "";

class AverageColorGPU {
 public:
  AverageColorGPU();
  ~AverageColorGPU();

  int init(GLuint inputTex);
  int reinit();
  void calculate();
  int shutdown();

 public:
  GLuint frag;
  GLuint vert;
  GLuint prog;
  GLuint vao;
  GLuint input_tex;

  GLuint fbo;
  GLuint output_tex;
  unsigned char* colors;
  GLint viewport[4];                     /* we read the viewport so we can reset it after calculating the average colors. */

  gfx::AsyncDownload async_download;     /* used to transfer the calculated average colors back to the cpu. */ 
};

#endif
