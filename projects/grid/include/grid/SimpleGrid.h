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

  
  class SimpleGrid {
  public:
    SimpleGrid();
    ~SimpleGrid();
    int init(SimpleSettings cfg);
    void update(float dt);
    void draw();
    int shutdown();
      
    int addImage(SimpleImage img);
    int prepare();
    int flip();
    bool isFull();

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
    size_t num_cells; /* how many cells are in use. */
  };

  /* after calling flip, call prepare to prepare the new layer before it becomes visible */
  inline int SimpleGrid::prepare() {
    if (NULL == front_layer) {
      RX_ERROR("No front layer set.");
      return -1;
    }
    return front_layer->prepare();
  }

  inline bool SimpleGrid::isFull() {
    if (NULL == back_layer) {
      RX_ERROR("No back layer set.");
      return false;
    }
    return back_layer->isFull();
  }

  inline int SimpleGrid::flip() {
    if (front_layer == NULL && back_layer == NULL) {
      RX_ERROR("front_layer && back_layer are not set.");
      return -1;
    }

    if (NULL == front_layer) {
      /* this is for the first switch; by default front_layer is null */
      front_layer = back_layer;
      back_layer = &layer_b;
      back_layer->reset();
    }
    else {
      SimpleLayer* tmp = front_layer;
      front_layer = back_layer;
      back_layer = tmp;
      back_layer->reset();
    }
    return 0;
  }

} /* namespace grid */

#endif
