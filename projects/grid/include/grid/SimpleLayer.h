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
    int layer;                                                 /* layer in texture array */
    bool in_use;                                               /* when set to true the vertices needs to be updated. */

    /* image data. */
    unsigned char* pixels;
    int img_width;
    int img_height;
    int allocated;
    int nbytes;
    int channels;

    /* animations */
    SimpleTween<vec2> tween_size;
    SimpleTween<float> tween_x;
  };

  /* ---------------------------------------------------------------- */
  
  enum {
    SIMPLE_EVENT_SHOWN,                                    /* gets dispatched when all elements are shown */
    SIMPLE_EVENT_HIDDEN,                                   /* gets dispatched when all elements are hidden */
    SIMPLE_EVENT_FULL                                      /* gets dispatched when the layer has enough images to be shown */
  };

  enum {
    SIMPLE_STATE_NONE,
    SIMPLE_STATE_SHOWING,
    SIMPLE_STATE_VISIBLE,
    SIMPLE_STATE_HIDDEN,
    SIMPLE_STATE_HIDING
  };

  /* ---------------------------------------------------------------- */

  class SimpleLayer;
  typedef void(*on_layer_event_callback)(SimpleLayer* layer, int event);

  /* ---------------------------------------------------------------- */

  class SimpleLayer {
  public:
    SimpleLayer();
    ~SimpleLayer();
    int init(SimpleSettings cfg);
    void update();
    void updatePhysics(float dt);
    int shutdown();
    int addImage(SimpleImage img);
    void reset();                                       /* resets all in_use flags of the cells. */
    void show();
    void hide();
    int prepareShow();                                  /* setup everything before it will become visible */
    int prepareHide();
    bool isFull();                                      /* do we have enough images to switch ? */

  public:
    SimpleSettings settings;
    std::vector<SimpleCell> cells;
    size_t cell_dx;
    size_t nimages;                                     /* how many images were added since the last reset */
    GLuint tex;
    int state; 
    uint64_t timeout;

    on_layer_event_callback on_event;
    void* user;
  }; 

  /* ---------------------------------------------------------------- */

  inline bool SimpleLayer::isFull() {
    return nimages >= cells.size();
  }

} /* namespace grid */

#endif
