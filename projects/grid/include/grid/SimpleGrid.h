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
#ifndef ROXLU_SIMPLE_GRID_H
#define ROXLU_SIMPLE_GRID_H

#include <glad/glad.h>
#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#include <grid/SimpleSettings.h>
#include <grid/SimpleLayer.h>
#include <grid/SimpleTypes.h>

static const char* SG_VS = ""
  "#version 330\n"
  "uniform mat4 u_pm;"
  "layout (location = 0) in vec4 a_pos; " 
  "layout (location = 1) in vec2 a_size; "
  "layout (location = 2) in int a_layer; "
  "out vec2 v_tex;"
  "out float v_layer;" 
  ""
  "const vec2 pos[] = vec2[4]("
  "  vec2(-0.5,  0.5),  "
  "  vec2(-0.5, -0.5),  "
  "  vec2(0.5,   0.5),  "
  "  vec2(0.5,  -0.5)   "
  ");"
  ""
  " const vec2[] tex = vec2[4]( "
  "   vec2(0.0, 0.0),  "
  "   vec2(0.0, 1.0),  "
  "   vec2(1.0, 0.0),  "
  "   vec2(1.0, 1.0)   "
  ");"
  ""
  "void main() {"
  "  vec2 offset = pos[gl_VertexID];"
  "  gl_Position = u_pm * vec4(a_pos.x + (offset.x * a_size.x), "
  "                            a_pos.y + (offset.y * a_size.y),   "
  "                            0.0, "
  "                            1.0); "
  "  v_tex = tex[gl_VertexID];"
  "  v_layer = a_layer;"
  "}"
  "";

static const char* SG_FS = ""
  "#version 330\n"
  "uniform sampler2DArray u_tex;"
  "in vec2 v_tex;"
  "in float v_layer;"
  "layout ( location = 0 ) out vec4 fragcolor;"
  
  "void main() {"
  "  vec3 texcoord = vec3(v_tex.x, 1.0 - v_tex.y, v_layer); " 
  "  vec4 tc = texture(u_tex, texcoord);"
  "  fragcolor.rgba = vec4(tc.b, tc.g, tc.r, tc.a); "
  "}"
  "";

namespace grid { 

  class SimpleGrid;
  typedef void(*simplegrid_event_callback)(SimpleGrid* grid, SimpleLayer* layer, int event);

  class SimpleGrid {
  public:
    SimpleGrid();
    ~SimpleGrid();
    int addImage(SimpleImage img);
    int init(SimpleSettings cfg);
    void updatePhysics(float dt);
    void updateVertices(SimpleLayer* from, std::vector<SimpleVertex>& vertices);
    void update();
    void draw();
    int shutdown();
    bool isFull();                                                                   /* checks if the current back_layer has enough images to switch */

  public:
    SimpleSettings settings;
    SimpleLayer layer_a;
    SimpleLayer layer_b;
    SimpleLayer* front_layer;
    SimpleLayer* back_layer;

    GLuint vert;
    GLuint frag;
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    size_t vbo_allocated;
    size_t num_cells;                                                                 /* how many cells are in use. */

    simplegrid_event_callback on_event;                                               /* passes layer events through to a user. */
    void* user;                                                                       /* user data. */
  };

  inline bool SimpleGrid::isFull() {
    if (NULL == back_layer) {
      RX_ERROR("Back layer not set.");
      return false;
    }
    return back_layer->isFull();
  }

} /* namespace grid */

#endif
