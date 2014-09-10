#ifndef ROXLU_SIMPLE_LAYER_H
#define ROXLU_SIMPLE_LAYER_H

#include <grid/SimpleSettings.h>
#include <grid/SimpleTypes.h>

#include <glad/glad.h>
#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>

namespace grid {

  /* ---------------------------------------------------------------- */

  class SimpleCell {
  public:
    SimpleCell();
    ~SimpleCell();
  public:
    int row;
    int col;
    int x;
    int y;
    int layer; /* layer in texture array */
    bool in_use; /* when set to true the vertices needs to be updated. */

    /* image data. */
    unsigned char* pixels;
    int img_width;
    int img_height;
    int allocated;
    int nbytes;
    int channels;

    /* animations */
    SimpleTween<vec2> tween_size;
  };

  /* ---------------------------------------------------------------- */

  class SimpleLayer {
  public:
    SimpleLayer();
    ~SimpleLayer();
    int init(SimpleSettings cfg);
    void update(float dt);
    void draw();
    int shutdown();
    int addImage(SimpleImage img);
    
    bool isFull(); /* will return true when all cells are in use */
    int prepare(); /* setup everything before it will become visible */
    void reset();  /* resets all in_use flags of the cells. */

  public:
    SimpleSettings settings;
    std::vector<SimpleCell> cells;
    size_t cell_dx;
    GLuint tex;
  }; 

  /* ---------------------------------------------------------------- */

} /* namespace grid */

#endif
