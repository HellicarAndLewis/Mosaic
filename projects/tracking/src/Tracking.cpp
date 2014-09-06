#include <tracking/Tracking.h>

namespace track {

  Tracking::Tracking() 
    :width(0)
    ,height(0)
    ,device(0)
    ,is_init(false)
    ,needs_update(false)
    ,tracker(NULL)
  {
  }

  Tracking::~Tracking() {
  }
  
  int Tracking::init(int dev, int w, int h) {

    device = dev;
    width = w;
    height = h;
    
    if (true == is_init) {
      RX_ERROR("Cannot initialize because we're already initialized.");
      return -100;
    }

    if (NULL != tracker) {
      RX_ERROR("Tracker is not NULL, not supposed to happen.");
      return -101;
    }

    if (0 == width || 0 == height) {
      RX_ERROR("Invalid width or height for the tracker. %d x %d", width, height);
      return -1;
    }

    if (0 > capture.open(device, width, height)) {
      RX_ERROR("Cannot open the video capture device: %d %d x %d", device, width, height);
      return -2;
    }

    if (0 > capture.start()) {
      RX_ERROR("Cannot start the video capture.");
      capture.close();
      return -3;
    }

    tracker = new Tracker(width, height, 5);
    if (NULL == tracker) {
      RX_ERROR("Cannot allocate the tracker");
      return -102;
    }

    if (0 != tiles.init()) {
      RX_ERROR("Cannot init the tiles, see error messages above.");
      capture.stop();
      capture.close();
      /* @todo - close tracker // free */
      return -104;
    }

    is_init = true;
    
    return 0;
  }

  int Tracking::shutdown() {

    if (false == is_init) {
      RX_ERROR("Cannot shutdown because we're not initialized yet.");
      return -1;
    }

    if (NULL != tracker) {
      delete tracker;
      tracker = NULL;
    }

    RX_ERROR("WE NEED TO SHUTDOWN THE WEBCAM AND TRACKER");

    if (0 != tiles.shutdown()) {
      RX_ERROR("Failed to shutdown the tiles renderer.");
    }

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

    tiles.update();
    needs_update = capture.needs_update;
  }

  /* @todo only update the bg buffer when we have a new frame. */
  void Tracking::draw() {

    tracker->beginFrame();
    {
      capture.update();
      capture.draw();
    }

    tracker->endFrame();
    tracker->apply();
    tracker->draw();

    tiles.draw();

    needs_update = false;
  }

} /* namespace t */
