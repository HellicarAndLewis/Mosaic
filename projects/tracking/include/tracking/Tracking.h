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

  Tracking
  ------

  Interactive layer for the Mosaic. 
    
*/
#ifndef ROXLU_TOPSHOP_TRACKING_H
#define ROXLU_TOPSHOP_TRACKING_H

#include <glad/glad.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#define ROXLU_USE_OPENGL
#include <tinylib.h>
#include <videocapture/CaptureGL.h>
#include <tracker/Tracker.h>
#include <tracking/Tiles.h>

namespace track {

  class Tracking {
  public:
    Tracking();
    ~Tracking();
    int init(int device, int width, int height);
    int shutdown();
    void update();
    void draw();

  public:
    int width; 
    int height;
    int device;
    ca::CaptureGL capture;
    Tracker* tracker;
    Tiles tiles;
    bool needs_update;
    bool is_init;
  };

} /* namespace track */

#endif
