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

  Tiles
  ------

  Draws the interactive layer on top of the mosaic.

  See here for tweening funcs: 
  http://www.timotheegroleau.com/Flash/experiments/easing_function_generator.htm 
    
*/

#ifndef ROXLU_TOPSHOP_TRACKING_TILES_H
#define ROXLU_TOPSHOP_TRACKING_TILES_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>
#include <mosaic/ImageLoader.h>
#include <pthread.h>
#include <deque>

static const char* TRACK_TILES_VS = ""
  "#version 330\n"
  "uniform mat4 u_pm;"
  "layout( location = 0 ) in vec4 a_pos;"
  "layout( location = 1 ) in vec2 a_size;"
  "layout( location = 2 ) in int a_layer;"
  "layout( location = 3 ) in float a_age;"
  "layout( location = 4 ) in float a_angle;"
  "out vec2 v_tex;"
  "out float v_layer;"

  "const vec2 pos[] = vec2[4]("
  "  vec2(-0.5,  0.5),  "
  "  vec2(-0.5, -0.5),  "
  "  vec2(0.5,   0.5),  "
  "  vec2(0.5,  -0.5)   "
  ");"

  " const vec2[] tex = vec2[4]( "
  "   vec2(0.0, 0.0),  "
  "   vec2(0.0, 1.0),  "
  "   vec2(1.0, 0.0),  "
  "   vec2(1.0, 1.0)   "
  ");"

  "void main() {"
  "  vec2 offset = pos[gl_VertexID];"
  "  float ct = cos(a_angle);"
  "  float st = sin(a_angle);"
  "  float xx = ct * offset.x - st * offset.y;"
  "  float yy = st * offset.x + ct * offset.y;"
  "  gl_Position = u_pm * vec4(a_pos.x + xx * a_size.x, "
  "                            a_pos.y + yy * a_size.y,   "
  "                            0.0, "
  "                            1.0); "
  "  v_tex = tex[gl_VertexID];"
  "  v_layer = a_layer;"
  "}"
  "";

static const char* TRACK_TILES_FS = ""
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

namespace track {


  /* ---------------------------------------------------------------- */

  struct Tween {
    void reset();
    void update(float now);
    void set(float duration, float start, float end);

    float start_time;
    float t;                                /* current time [0-1] */  
    float d;                                /* duration */
    float b;                                /* start value */
    float c;                                /* change */
    float value;                            /* last updated value */
  };

  /* ---------------------------------------------------------------- */

  class Image {
  public:
    Image();
    ~Image();
  public:
    unsigned char* pixels;
    size_t allocated; /* how many bytes we can store in pixels. */
    bool is_free;
    float layer;
    
    int x;  /* where to position when created, used by the boink effect ? */
    int y;  /* where to position when created, used by the boink effect ? */
    int mode; /* what image mode */
    Tween tween_size;
    Tween tween_angle;
    Tween tween_x;
    Tween tween_y;
  };

  /* ---------------------------------------------------------------- */

  enum {
    IMAGE_MODE_NONE,
    IMAGE_MODE_BOINK, /* an image is shown at x/y and grows and then hides again */
    IMAGE_MODE_FLY,   /* an image flies into view to x/y and rotates to the set destination angle */
  };

  struct ImageOptions {
    int mode; 
    int x;
    int y;
    std::string filepath;
    Tween tween_size;
    Tween tween_angle;
    Tween tween_x;
    Tween tween_y;
  };

  /* ---------------------------------------------------------------- */


  enum {
    VERTEX_STATE_NONE,
    VERTEX_STATE_TWEEN_IN,
    VERTEX_STATE_TWEEN_OUT
  };

  struct Vertex {
    vec2 position;
    vec2 size;
    int layer;                               /* what texture array layer, indicates what texture to use, < 0 means the vertex is not used. */
    float age;
    float angle;                             /* the z-rotation. */
  };

  inline void Tween::set(float duration, float start, float change) {
    d = duration;
    b = start;
    c = change;
    start_time = rx_millis();
    t = 0.0f;
    value = 0.0f;
  }

  struct Particle {
    vec2 position;
    vec2 size;
    int layer;
    float age;
    float angle;

    uint8_t state;
    int mode;                                /* what mode, e.g. how the image is presented on screen */
    Tween tween_size;
    Tween tween_angle;
    Tween tween_x;
    Tween tween_y;
   };

  /* ---------------------------------------------------------------- */

  class Tiles {
  public:
    Tiles();
    ~Tiles();
    int init(int texW, int texH);
    void update();
    void draw();
    int shutdown();
    int load(ImageOptions& options);
    Image* getFreeImage();                      /* the images are loaded in a separate thread and when loaded the data is copied to the main thread so we can update the texture data. We reuse preallocated Images for this. Returns NULL on error */
    void lock();
    void unlock();
    void updateVertexState();
    int showTileAtPosition(std::string filename, int x, int y);

  public:
    bool is_init;
    int tex_width;
    int tex_height;
    int tex_ntotal;                            /* how many textures are allowed to be stored on the gpu in total */ 
    mos::ImageLoader img_loader;
    std::vector<Image*> images;
    std::vector<Vertex> vertices;
    std::vector<Particle*> particles;
    std::deque<int> free_layers;

    /* gl */
    GLuint frag;
    GLuint vert;
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    GLuint tex;

    /* threading - we need to sync with the image loader thread. */
    bool must_update;                       /* is set to true when an new image was loaded. */
    pthread_mutex_t mutex;
  };

  inline void Tiles::lock() {
    int r = pthread_mutex_lock(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot lock: %s", strerror(r));
    }
  }

  inline void Tiles::unlock() {
    int r = pthread_mutex_unlock(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot unlock: %s", strerror(r));
    }
  }

} /* namespace track */

#endif
