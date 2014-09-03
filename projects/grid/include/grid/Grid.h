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

  Grid
  ----

  Watches the given directory for images that are drawn in a grid. The
  grid slowly moves from left to right, or right to left. We watch for
  new files in a directory and when a new file is found, it will be loaded
  in a separate thread. Once loaded we find the best position where we 
  can store the new image. 

  We're combining all the images into one atlas texture so we wonly had to
  deal with one texture and don't need to bind and manage multiple textures.

  This code doubles the number of columns (see init); see an explanation why 
  we do that  here: http://i.imgur.com/m5RKNDQ.png

 */
#ifndef ROXLU_GRID_H
#define ROXLU_GRID_H

#include <deque>
#include <vector>
#include <mosaic/DirWatcher.h>
#include <mosaic/ImageLoader.h>
#include <glad/glad.h>
#include <pthread.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>

#define CELL_STATE_NONE 0x00
#define CELL_STATE_LOADED 0x01                /* the cell image is read from disk */
#define CELL_STATE_RESERVED 0x02              /* the cell is reserved and should be filled when the file is loaded. */
#define CELL_STATE_IN_USE 0x03                /* the cell is in use */

#define GRID_DIR_RIGHT 0x01
#define GRID_DIR_LEFT 0x02

namespace grid {

  static const char* GRID_VS = ""
    "#version 330\n"
    "uniform mat4 u_pm;"
    "uniform vec2 u_pos;"
    "layout( location = 0 ) in vec4 a_pos;"
    "layout( location = 1 ) in float a_size;"
    "layout( location = 2 ) in float a_row;"
    "layout( location = 3 ) in float a_col;"

    "out float v_row;"
    "out float v_col;"
    "out vec2 v_tex;"
    "out vec2 v_pos;"

    "const vec2 pos[] = vec2[4]("
    "  vec2(-0.5,  0.5),  "
    "  vec2(-0.5, -0.5),  "
    "  vec2(0.5,   0.5),  "
    "  vec2(0.5,  -0.5)   "
    ");"

    /* we have to use this somewhat `odd` texture coordinates because else you'll get overlapping between columns. */
    " const vec2[] tex = vec2[4]( "
    "   vec2(0.01, 0.01),  "
    "   vec2(0.01, 0.99),  "
    "   vec2(0.99, 0.01),  "
    "   vec2(0.99, 0.99)   "
    ");"

    "void main() {"
    "  vec2 offset = pos[gl_VertexID];"
    "  gl_Position = u_pm * vec4(u_pos.x + a_pos.x + (offset.x * a_size), "
    "                            u_pos.y + a_pos.y + (offset.y * a_size),  "
    "                            0.0, "
    "                            1.0); "
    "  v_tex = tex[gl_VertexID];"
    "  v_pos = offset;"
    "  v_row = a_row; "
    "  v_col = a_col; "
    "}"
    "";

  static const char* GRID_FS = ""
    "#version 330\n"
    "uniform sampler2D u_tex;"
    "uniform float u_scalex;"
    "uniform float u_scaley;"
    "layout (location = 0) out vec4 fragcolor;"
    "in float v_col;"
    "in float v_row;"
    "in vec2 v_tex;"
    "in vec2 v_pos;"

    "void main() {"
    "   fragcolor = vec4(1.0, 0.0, 0.0, 1.0);"
    "   vec2 texcoord = vec2(v_tex.x * u_scalex  + v_col * u_scalex, 1.0 - (v_tex.y * u_scaley + v_row * u_scaley));"
    "   vec4 tc = texture(u_tex, texcoord);"
    "   fragcolor.rgb = tc.rgb;"
    "}"
    "";

  /* ---------------------------------------------------------------------------------- */

  struct Vertex {
    Vertex();
    ~Vertex();

    vec2 pos;
    float size;
    float row;
    float col;
  };

  struct Cell {
    Cell();
    ~Cell();

    int row;
    int col;
    int dx;
    uint64_t time_added;
    int state;

    /* will hold the pixel data, the buffer capacity is img_width * img_height * 4 */
    unsigned char* pixels;
    int capacity;
  };

  /* ---------------------------------------------------------------------------------- */

  struct Source {
    std::string filepath;
    uint64_t mtime;
  };

  /* ---------------------------------------------------------------------------------- */

  class Grid {

  public:
    Grid(int direction);
    ~Grid();
    int init(std::string path, int imgWidth, int imgHeight, int rows, int cols);
    void update();
    void draw();
    int shutdown();
    
    void lock();
    void unlock();
    bool getUsableCell(size_t& dx, int currState, int newState);

  public:

    mos::ImageLoader img_loader;
    mos::DirWatcher dir_watcher;

    bool is_init;                                 /* is set to true when init() completed successfully */
    int direction;                                /* the direction into wich the stream with images moves; this also has influence on the offset. e.g. when direction is right, the offset.x is the right edge. */
    int img_width;                                /* width of an image, for now this must be similar as the img_height */ 
    int img_height;                               /* height of an image, see img_width. */
    int rows;                                     /* number of rows. */
    int cols;                                     /* number of columns, but note that we double the number, which is necessary to smoothly, continuously scroll the images. */

    GLuint tex_id;
    GLsizei tex_width;
    GLsizei tex_height;

    std::vector<Cell> cells;                      /* keeps state for the separate cells */
    std::deque<Source> sources;                   /* a list with source files */
    std::vector<Vertex> vertices;                 /* the data for the VBO */
    std::vector<size_t> index_order;              /* what grid element do we need to fill next, depending on the direction the order will be different. */
    pthread_mutex_t mutex;                        /* used to protect sources */
    int col_hidden;                               /* which column is currently hidden? this is always the 'last' column the scrolled out of view and this column can be reused. */
    int prev_col_hidden;                          /* previously column that was set to hidden; used to track if a the hidden column changed */

    /* gl */
    mat4 pm;
    GLuint vert;
    GLuint frag;
    GLuint prog;
    GLuint vao;
    GLuint vbo;
    GLint u_pos;                                  /* uniform to the position */
    GLint vp[4];                                  /* viewport */

    vec2 pos_a;                                   /* position of first set */
    vec2 pos_b;                                   /* position of second set */
    vec2 offset;                                  /* top left position where we start drawing (relative to the current position) */
    vec2 padding;                                 /* padding between cells. */
  }; 

  inline void Grid::lock() {
    if (0 != pthread_mutex_lock(&mutex)) {
      RX_ERROR("Something went wrong when trying to lock the mutex");
    }
  }

  inline void Grid::unlock() {
    if (0 != pthread_mutex_unlock(&mutex)) {
      RX_ERROR("Something went wrong when trying to unlock the mutex.");
    }
  }

} /* namespace grid */

#endif
