#include <tracking/Tracking.h>

namespace track {

  Tracking::Tracking() 
    :width(0)
    ,height(0)
    ,device(0)
    ,is_init(false)
    ,needs_update(false)
#if USE_TRACKER
    ,tracker(NULL)
#endif
  {
  }

  Tracking::~Tracking() {
  }
  
  int Tracking::init(int dev, int w, int h) {
    
    if (true == is_init) {
      RX_ERROR("Cannot initialize because we're already initialized.");
      return -100;
    }

#if USE_TRACKER
    if (NULL != tracker) {
      RX_ERROR("Tracker is not NULL, not supposed to happen.");
      return -101;
    }
#endif

    device = dev;
    width = w;
    height = h;

#if USE_TRACKER    
    tracker = new Tracker(width, height, 5);
    if (NULL == tracker) {
      RX_ERROR("Cannot allocate the tracker");
      return -102;
    }
#endif

    if (0 == width || 0 == height) {
      RX_ERROR("Invalid width or height for the tracker. %d x %d", width, height);
      return -1;
    }

    if (0 > capture.open(device, width, height)) {
      RX_ERROR("Cannot open the video capture device: %d %d x %d", device, width, height);
      return -2;
    }

    capture.listCapabilities(device);

    if (0 > capture.start()) {
      RX_ERROR("Cannot start the video capture.");
      capture.close();
      return -3;
    }

    is_init = true;
    
    return 0;
  }

  int Tracking::shutdown() {

    if (false == is_init) {
      RX_ERROR("Cannot shutdown because we're not initialized yet.");
      return -1;
    }

#if USE_TRACKER
    if (NULL != tracker) {
      delete tracker;
      tracker = NULL;
    }
#endif

    is_init = false;

    return 0;
  }

  void Tracking::update() {

#if !defined(NDEBUG) 
    if (false == is_init) {
      RX_ERROR("You're trying to update the tracker but it's not initialized.");
      return;
    }
#endif

    needs_update = capture.needs_update;
    RX_VERBOSE("Update: %d", needs_update);
    capture.update();
  }

  void Tracking::draw() {

#if USE_TRACKER
      tracker->beginFrame();
        capture.draw();
      tracker->endFrame();
      tracker->apply();
      tracker->draw();
#else 
      capture.draw();
#endif

      needs_update = false;

      /*
    if (true == needs_update) {
      tracker->beginFrame();
        capture.draw();
      tracker->endFrame();
      tracker->apply();
      needs_update = false;
    }
    else {
      capture.draw();
      tracker->draw();
    }
      */

  }

} /* namespace t */
