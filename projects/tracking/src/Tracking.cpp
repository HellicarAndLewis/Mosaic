#include <tracking/Tracking.h>

namespace track {

  /* ---------------------------------------------------------------------------------- */

  static void tracking_on_activate_cell(int i, int j, void* user); /* is called by the InteractiveGrid when we need to make a specific cell active */

  /* ---------------------------------------------------------------------------------- */

  Tracking::Tracking() 
    :width(0)
    ,height(0)
    ,device(0)
    ,is_init(false)
    ,needs_update(false)
    ,tracker(NULL)
    ,on_activate(NULL)
    ,user(NULL)
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

    if (0 != tiles.init(284, 347)) {
      RX_ERROR("Cannot init the tiles, see error messages above.");
      capture.stop();
      capture.close();
      /* @todo - close tracker // free */
      return -104;
    }

    if (0 != interactive_grid.init(tracker, &tiles)) {
      RX_ERROR("Cannot init the interactive grid.");
      capture.stop();
      capture.close();
      tiles.shutdown();
      delete tracker;
      tracker = NULL;
      return -105;
    }

    interactive_grid.user = this;
    interactive_grid.on_activate = tracking_on_activate_cell;

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

    if (0 != tiles.shutdown()) {
      RX_ERROR("Failed to shutdown the tiles renderer.");
    }

    if (0 != interactive_grid.shutdown()) {
      RX_ERROR("Failed to shutdown the interactive grid.");
    }

    capture.stop();
    capture.close();

    interactive_grid.user = NULL;
    interactive_grid.on_activate = NULL;
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

    interactive_grid.update();
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
    //    tracker->draw();

    tiles.draw();

    needs_update = false;
  }

  /* ---------------------------------------------------------------------------------- */

  static void tracking_on_activate_cell(int i, int j, void* user) {

    Tracking* tracking = static_cast<Tracking*>(user);
    if (NULL == tracking) {
      RX_ERROR("Cannot get handle to Tracking.");
      return;
    }
    if (NULL == tracking->on_activate) {
      RX_ERROR("on_activate not set in Tracking.");
      return;
    }

    tracking->on_activate(i, j, tracking->user);
  }

} /* namespace t */
