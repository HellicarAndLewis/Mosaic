#include <tracking/Tracking.h>

namespace track {

  /* ---------------------------------------------------------------------------------- */

  static void tracking_on_activate_cell(int i, int j, void* user); /* is called by the InteractiveGrid when we need to make a specific cell active */

  /* ---------------------------------------------------------------------------------- */

  TrackingSettings::TrackingSettings() 
    :webcam_device(-1)
    ,webcam_width(0)
    ,webcam_height(0)
    ,tile_width(0)
    ,tile_height(0)
    ,tile_nlayers(0)
  {
  }

  TrackingSettings::~TrackingSettings() {
    webcam_device = -1;
    webcam_width = 0;
    webcam_height = 0;
    tile_width = 0;
    tile_height = 0;
    tile_nlayers = 0;
  }

  bool TrackingSettings::validate() {
    if (-1 == webcam_device) { RX_ERROR("Invalid webcam_device"); return false; } 
    if (0 == webcam_width) { RX_ERROR("Invalid webcam_width"); return false; } 
    if (0 == webcam_height) { RX_ERROR("Invalid webcam_height"); return false; } 
    if (0 == tile_width) { RX_ERROR("Invalid tile_with"); return false; } 
    if (0 == tile_height) { RX_ERROR("Invalid tile_height"); return false; } 
    if (0 == tile_nlayers) { RX_ERROR("Invalid tile_nlayers"); return false; }
    return true;
  }

  /* ---------------------------------------------------------------------------------- */

  Tracking::Tracking() 
    :is_init(false)
    ,needs_update(false)
#if USE_TRACKER
    ,tracker(NULL)
#endif
    ,on_activate(NULL)
    ,user(NULL)
  {
  }

  Tracking::~Tracking() {

    if (true == is_init) {
      shutdown();
    }

    is_init = false;
    user = NULL;
    on_activate = NULL;
    needs_update = false;
  }
  
  int Tracking::init(TrackingSettings cfg) {

    settings = cfg;
    if (false == settings.validate()) {
      return -1;
    }

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

    if (0 > capture.open(settings.webcam_device, settings.webcam_width, settings.webcam_height)) {
      RX_ERROR("Cannot open the video capture device: %d %d x %d", settings.webcam_device, settings.webcam_width, settings.webcam_height);
      return -2;
    }

    if (0 > capture.start()) {
      RX_ERROR("Cannot start the video capture.");
      capture.close();
      return -3;
    }

#if USE_TRACKER
    tracker = new Tracker(settings.webcam_width, settings.webcam_height, 5);
    if (NULL == tracker) {
      RX_ERROR("Cannot allocate the tracker");
      return -102;
    }
#endif

#if USE_TILES
    if (0 != tiles.init(settings.tile_width, settings.tile_height, settings.tile_nlayers)) {
      RX_ERROR("Cannot init the tiles, see error messages above.");
      capture.stop();
      capture.close();
      /* @todo - close tracker // free */
      return -104;
    }
#endif

#if USE_TRACKER && USE_TILES
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
#endif

    capture.flip(true, true);

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

#if USE_TILES
    if (0 != tiles.shutdown()) {
      RX_ERROR("Failed to shutdown the tiles renderer.");
    }
#endif

#if USE_TRACKER
    if (0 != interactive_grid.shutdown()) {
      RX_ERROR("Failed to shutdown the interactive grid.");
    }
    interactive_grid.user = NULL;
    interactive_grid.on_activate = NULL;
#endif

    is_init = false;

    capture.stop();
    capture.close();


    return 0;
  }

  void Tracking::update() {

#if !defined(NDEBUG) 
    if (false == is_init) {
      RX_ERROR("You're trying to update the tracker but it's not initialized.");
      return;
    }
#endif

#if USE_TRACKER
    interactive_grid.update();
#endif

#if USE_TILES
    tiles.update();
#endif

    needs_update = capture.needs_update;
  }

  /* @todo only update the bg buffer when we have a new frame. */
  void Tracking::draw() {

#if USE_TRACKER
    tracker->beginFrame();
    {
      capture.update();
      capture.draw();
    }

    tracker->endFrame();
    tracker->apply();
    // tracker->draw();
#else 
    capture.update();
    capture.draw();
#endif

#if USE_TILES
    tiles.draw();
#endif

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
