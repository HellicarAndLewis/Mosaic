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

#define USE_TRACKER 1
#define USE_BG 0

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
#if USE_TRACKER
    Tracker* tracker;
#endif
#if USE_BG 
    BackgroundBuffer* bg;
    Painter painter;
#endif
    bool needs_update;
    bool is_init;
  };

} /* namespace track */

#endif
