#include <featurex/Config.h>
#include <topshop/Config.h>
#include <topshop/TopShop.h>

namespace top {

  /* ------------------------------------------------------------------------- */
  static void on_new_file(ImageCollector* col, CollectedFile& file);         /* gets called when a file is ready to be shown in the mosaic. */
  static void topshop_activate_cell(int i, int j, void* user);               /* gets called when we need to show the bigger version of the given cell, is called by the `interactive_grid` member of the Tracking layer. */
  /* ------------------------------------------------------------------------- */
  
  TopShop::TopShop() 
  {
  }

  TopShop::~TopShop() {

  }

  int TopShop::init() {

    int r = 0;
    
    /* make sure the webcam settings are correct. */
    if (0 != mos::config.validateWebcam()) {
      RX_ERROR("Webcam settings are incorrect");
      return -98;
    }
    
    /* initialize the tracker. */
    r = tracking.init(mos::config.webcam_device, mos::config.webcam_width, mos::config.webcam_height);
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
    tracking.update();
    remote_state.update();
  }

  void TopShop::draw() {

    mosaic.draw(top::config.mosaic_x, top::config.mosaic_y, top::config.mosaic_width, top::config.mosaic_height);

    if (1 == top::config.is_debug_draw) {
      mosaic.debugDraw();
    }

    tracking.draw();
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

    fex::Descriptor desc;
    if (0 != shop->mosaic.featurex.getDescriptorGPU(i, j, desc)) {
      RX_ERROR("Cannot get a descriptor for col: %d and row :%d", i, j);
      return;
    }

    std::string filepath = fex::config.resized_filepath +"/" +desc.getFilename();
    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Cannot find the file: %s for the bigger grid version", filepath.c_str());
      return;
    }

    float col_w = float(mos::config.window_width) / fex::config.cols;
    float col_h = float(mos::config.window_height) / fex::config.rows;
    track::ImageOptions img_opt;
    img_opt.x = col_w * i;
    img_opt.y = col_h * j;
    img_opt.filepath = filepath;
    if (0 != shop->tracking.tiles.load(img_opt)) {
      RX_ERROR("Something went wrong while trying to load %s", filepath.c_str());
    }

    RX_VERBOSE("i: %d, j: %d --> %s", i, j, desc.getFilename().c_str());
  }

  /* ------------------------------------------------------------------------- */
  
} /* namespace top */
