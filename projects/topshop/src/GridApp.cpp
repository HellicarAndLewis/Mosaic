#include <topshop/GridApp.h>

namespace top {
  
  /* ------------------------------------------------------------------------- */
  
  static void on_new_file(ImageCollector* col, CollectedFile& file);         /* gets called when a file is ready to be shown in the mosaic. */

  /* ------------------------------------------------------------------------- */


  GridAppSettings::GridAppSettings() {
    reset();
  }

  GridAppSettings::~GridAppSettings() {
    reset();
  }

  void GridAppSettings::reset() {
    image_width = 0;
    image_height = 0;
    grid_rows = 0;
    grid_cols = 0;
    image_path.clear();
    watch_path.clear();
  }

  int GridAppSettings::validate() {
    if (0 == image_width) { RX_ERROR("image_width is invalid"); return -1; }
    if (0 == image_height) { RX_ERROR("image_height is invalid"); return -2; }
    if (0 == grid_cols) { RX_ERROR("grid_cols is invalid"); return -3; }
    if (0 == grid_rows) { RX_ERROR("grid_rows is invalid"); return -4; }
    if (0 == image_path.size()) { RX_ERROR("image_path  is invalid"); return -5; }
    if (0 == watch_path.size()) { RX_ERROR("watch_path  is invalid"); return -6; }
    return 0;
  }

  /* ------------------------------------------------------------------------- */


  GridApp::GridApp(int dir) 
    :grid(dir)
  {
  }

  GridApp::~GridApp() {
  }

  // int GridApp::init(std::string path, int imgW, int imgH, int rows, int cols) {
  int GridApp::init(GridAppSettings& cfg) {
    int r = 0; 

    if (0 != cfg.validate()) {
      return -1;
    }

    settings = cfg;

    /* init the grid. */
    r = grid.init(cfg.image_path, cfg.image_width, cfg.image_height, cfg.grid_rows, cfg.grid_cols);
    if (0 != r) {
      RX_ERROR("Cannot initialize the grid: %d.", r);
      return -2;
    }

    /* init the dir watcher. */
    r = img_collector.init(cfg.watch_path); 
    if (0 != r) {
      RX_ERROR("Cannot start the image collector: %d.", r);
      grid.shutdown();
      return -3;
    }

    img_collector.on_file = on_new_file;
    img_collector.user = this;

    /* init the image processor */
    r = img_processor.init();
    if (r != 0) {
      RX_ERROR("Cannot init the image processor: %d", r);
      grid.shutdown();
      img_collector.shutdown();
      return -4;
    }
    
    return 0;
  }

  int GridApp::shutdown() {

    RX_ERROR("Need to add error checking!");

    img_collector.shutdown();
    img_processor.shutdown();
    grid.shutdown();
    return 0;
  }

  void GridApp::update() {
    img_collector.update();
    grid.update();
  }

  void GridApp::draw() {
    grid.draw();
  }

  static void on_new_file(ImageCollector* col, CollectedFile& file) {

    if (NULL == col) {
      RX_ERROR("Invalid ImageCollector ptr.");
      return;
    }
    
    GridApp* app = static_cast<GridApp*>(col->user);
    if (NULL == app) {
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

    RX_VERBOSE("Got a new file: %s", file.filename.c_str());

    if (GRID_DIR_RIGHT == app->grid.direction) {
      file.type = COL_FILE_TYPE_LEFT_GRID;
    }
    else if (GRID_DIR_LEFT == app->grid.direction) {
      file.type = COL_FILE_TYPE_RIGHT_GRID;
    }
    else {
      RX_ERROR("Unhanded grid direction.Ignoring file: %s", file.filename.c_str());
      return;
    }

    if (0 != app->img_processor.process(file)) {
      RX_ERROR("Something went wrong in the image processor; see log above.");
    }
  } 

} /* namespace top */
