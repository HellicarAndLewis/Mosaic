#include <featurex/Config.h>
#include <topshop/Config.h>
#include <topshop/TopShop.h>

namespace top {

  /* ------------------------------------------------------------------------- */
  static void on_new_file(ImageCollector* col, CollectedFile& file);         /* gets called when a file is ready to be shown in the mosaic. */
  static void topshop_activate_cell(int i, int j, void* user);               /* gets called when we need to show the bigger version of the given cell, is called by the `interactive_grid` member of the Tracking layer. */
  /* ------------------------------------------------------------------------- */
  
  TopShop::TopShop() 
#if USE_POLAROID
    :polaroid_timeout(0)
#endif
  {
  }

  TopShop::~TopShop() {
#if USE_POLAROID
    polaroid_timeout = 0;
#endif
  }

  int TopShop::init() {

    int r = 0;
    
    /* make sure the webcam settings are correct. */
    if (0 != mos::config.validateWebcam()) {
      RX_ERROR("Webcam settings are incorrect");
      return -98;
    }
    
    /* initialize the tracker. */
    track::TrackingSettings cfg;
    cfg.webcam_device = mos::config.webcam_device;
    cfg.webcam_width = mos::config.webcam_width;
    cfg.webcam_height = mos::config.webcam_height;

#if USE_POLAROID
    cfg.tile_width = 568;
    cfg.tile_height = 568;
    cfg.tile_nlayers = 10;
#else
    cfg.tile_width = 255; // fex::config.file_tile_width; 
    cfg.tile_height = 255; // fex::config.file_tile_height; 
    cfg.tile_nlayers = 150;
#endif

    r = tracking.init(cfg);
    if (0 != r) {
      RX_ERROR("Cannot initialize the tracker.");
      return -99;
    }

    /* init the mosaic. */
    r = mosaic.init();
    if (0 != r) {
      RX_ERROR("Cannot init mosiac: %d", r);
      tracking.shutdown();
      return -101;
    }

    /* init the dir watcher. */
    r = img_collector.init(fex::config.raw_filepath);
    if (0 != r) {
      RX_ERROR("Cannot start the image collector: %d.", r);
      mosaic.shutdown();
      tracking.shutdown();
      return -102;
    }

    r = remote_state.init();
    if (0 != r) {
      RX_ERROR("Cannot init the remote state: %d", r);
      mosaic.shutdown();
      tracking.shutdown();
      img_collector.shutdown();
      return -103;
    }

    img_collector.user = this;
    img_collector.on_file = on_new_file;
    tracking.user = this;
    tracking.on_activate = topshop_activate_cell;

    return 0;
  }

  int TopShop::shutdown() {
    int r = 0;

    r = mosaic.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the mosaic: %d", r);
    }

    r = img_collector.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to shutdown the image collector: %d.", r);
    }

    r = remote_state.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to shutdown the remote state: %d", r);
    }
    
    return 0;
  }

  void TopShop::update() {

    img_collector.update();
    mosaic.update();
    remote_state.update();

    /* get remote settings. */
    remote_state.getRemoteSettings(remote_settings);
    top::config.is_debug_draw = (0 == remote_settings.show_mosaic) ? 1 : 0;

    /* only track when we're not showing the video. */
    if (0 == top::config.is_debug_draw) {
      tracking.update();
    }
  }

  void TopShop::draw() {

    mosaic.draw(top::config.mosaic_x, top::config.mosaic_y, top::config.mosaic_width, top::config.mosaic_height);

    /* "debug draw" shows the video */
    if (1 == top::config.is_debug_draw) {
      mosaic.debugDraw();
    }
    else {
      tracking.draw();
    }
  }

  /* ------------------------------------------------------------------------- */

  static void on_new_file(ImageCollector* col, CollectedFile& file) {

    if (NULL == col) {
      RX_ERROR("Invalid ImageCollector ptr.");
      return;
    }
    
    TopShop* shop = static_cast<TopShop*>(col->user);
    if (NULL == shop) {
      RX_ERROR("Cannot cast the user member of the ImageCollector.");
      return;
    }
    
    std::string filepath = file.dir +"/" +file.filename;

#if !defined(NDEBUG)
    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Filepath doesn't exists: %s", filepath.c_str());
      return;
    }
#endif

    /* @todo - we assume the file is correct here, we may add validation here to be sure */
    RX_VERBOSE("Got a mosaic file: %s", file.filename.c_str());

    file.type = COL_FILE_TYPE_RAW;

    if (0 != shop->mosaic.analyzeCPU(filepath)) {
      RX_ERROR("Failed to add a new file for the cpu analyzer. Check messages above");
    }

  } 

  static void topshop_activate_cell(int i, int j, void* user) {

#if !defined(NDEBUG)
    if (0 == mos::config.window_width || 0 == mos::config.window_height || 0 == fex::config.cols || 0 == fex::config.rows) {
      RX_ERROR("Invalid config; check window_width, window_height, fex::config.cols, fex::config.rows");
      return;
    }
#endif

    TopShop* shop = static_cast<TopShop*>(user);
    if (NULL == shop) {
      RX_ERROR("Cannot get a valid TopShop handle");
      return;
    }

#if USE_POLAROID
    /* check if we can show the next polaroid */
    uint64_t now = rx_hrtime();
    if (0 != shop->polaroid_timeout && now < shop->polaroid_timeout) {
      return;
    }
    uint64_t polaroid_delay = 1500e6; /* XXXe6 millis seconds */
    shop->polaroid_timeout = now + polaroid_delay;
#endif

    fex::Descriptor desc;
    if (0 != shop->mosaic.featurex.getDescriptorGPU(i, j, desc)) {
      RX_ERROR("Cannot get a descriptor for col: %d and row :%d", i, j);
      return;
    }

    if (0 == desc.getFilename().size()) {
      RX_ERROR("Descriptor has no filename set!");
      return;
    }

    if (0 != shop->tracking.hasFreeLayer()) {
      RX_VERBOSE("No free layer, so ignoring this one..");
      return;
    }

#if USE_POLAROID
    std::string filepath = top::config.polaroid_filepath +"/" +desc.getFilename();
#else
    std::string filepath = top::config.polaroid_filepath +"/" +desc.getFilename();
#endif

    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Cannot find the file: %s for the bigger grid version", filepath.c_str());
      return;
    }

    float col_w = float(mos::config.window_width) / fex::config.cols;
    float col_h = float(mos::config.window_height) / fex::config.rows;

    track::ImageOptions img_opt;

#if USE_POLAROID
    img_opt.filepath = filepath;
    img_opt.tween_x.set(2.0f, mos::config.window_width * 0.5, rx_random(-600, 600));
    img_opt.tween_y.set(2.0f, mos::config.window_height, rx_random(-400, -700));

    float start_angle = rx_random(-PI, PI);
    img_opt.tween_angle.set(1.3f,start_angle , (start_angle * -1) + rx_random(0.3)); //rx_random(-(0.5 *HALF_PI), 0.5 * HALF_PI));
    img_opt.tween_size.set(1.3f, 0.0f, 400.0f);
    img_opt.mode = track::IMAGE_MODE_FLY;
#else
    img_opt.x = col_w * i;
    img_opt.y = col_h * j;
    img_opt.filepath = filepath;
    img_opt.mode = track::IMAGE_MODE_BOINK;
    img_opt.tween_size.set(0.6f, 0.0f, rx_random(150.0f, 195.0f));

    float angle_range = PI * 0.25;
    float start_angle = rx_random(-angle_range, angle_range);
    img_opt.tween_angle.set(1.3f, start_angle , (start_angle * -1) + rx_random(0.3)); //rx_random(-(0.5 *HALF_PI), 0.5 * HALF_PI));
#endif

    if (0 != shop->tracking.load(img_opt)) {
      RX_ERROR("Something went wrong while trying to load %s", filepath.c_str());
    }

    RX_VERBOSE("i: %d, j: %d --> %s", i, j, desc.getFilename().c_str());
  }

  /* ------------------------------------------------------------------------- */
  
} /* namespace top */
