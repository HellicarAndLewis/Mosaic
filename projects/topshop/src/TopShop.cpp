#include <featurex/Config.h>
#include <topshop/Config.h>
#include <topshop/TopShop.h>

namespace top {

  /* ------------------------------------------------------------------------- */
  
  static void on_new_file(ImageCollector* col, const CollectedFile& file);         /* gets called when a file is ready to be shown in the mosaic. */

  /* ------------------------------------------------------------------------- */
  
  TopShop::TopShop() 
#if USE_GRID
    :left_grid(GRID_DIR_RIGHT)
    ,right_grid(GRID_DIR_LEFT)
#endif
  {
  }

  TopShop::~TopShop() {

  }

  int TopShop::init() {
    int r = 0;

    /* init the mosaic. */
    r = mosaic.init();
    if (0 != r) {
      RX_ERROR("Cannot init mosiac: %d", r);
      return -101;
    }

    /* init the image collector */
    r = img_collector.init();
    if (0 != r) {
      RX_ERROR("Cannot init the image collector: %d", r);
      mosaic.shutdown();
      return -102;
    }

    r = img_processor.init();
    if (0 != r) {
      RX_ERROR("Cannot init the image processor: %d", r);
      mosaic.shutdown();
      img_collector.shutdown();
      return -103;
    }

    /* set the image collector callbacks. */
    img_collector.user = this;
    img_collector.on_raw_file = on_new_file;
    img_collector.on_left_grid_file = on_new_file;
    img_collector.on_right_grid_file = on_new_file;


#if USE_GRID
    r = left_grid.init(top::config.left_grid_filepath, 
                       top::config.grid_file_width, 
                       top::config.grid_file_height, 
                       top::config.grid_rows, 
                       top::config.grid_cols);
    if (0 != r) {
      RX_ERROR("Cannot init the left grid.");
      img_collector.shutdown();
      mosaic.shutdown();
      return -105;
    }
    left_grid.offset.set(top::config.left_grid_x, top::config.left_grid_y);
    left_grid.padding.set(top::config.grid_padding_x, top::config.grid_padding_y);

    r = right_grid.init(top::config.right_grid_filepath, 
                        top::config.grid_file_width, 
                        top::config.grid_file_height, 
                        top::config.grid_rows, 
                        top::config.grid_cols);
    if (0 != r) {
      RX_ERROR("Cannot init the right grid.");
      left_grid.shutdown();
      img_collector.shutdown();
      mosaic.shutdown();
      return -106;
    }
    right_grid.offset.set(top::config.right_grid_x, top::config.right_grid_y);
    right_grid.padding.set(top::config.grid_padding_x, top::config.grid_padding_y);
#endif

    return 0;
  }

  int TopShop::shutdown() {
    int r = 0;

#if USE_GRID
    r = left_grid.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the left grid: %d", r);
    }
#endif

    r = mosaic.shutdown();
    if (0 != r) {
      RX_ERROR("Cannot shutdown the mosaic: %d", r);
    }

    r = img_collector.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to shutdown the image collector cleanly: %d", r);
    }

    r = img_processor.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to shutdown the image processor: %d", r);
    }

    return 0;
  }

  void TopShop::update() {

    img_collector.update();
    mosaic.update();

#if USE_GRID
    left_grid.update();
    right_grid.update();
#endif

  }

  void TopShop::draw() {
#if USE_GRID
    left_grid.draw();
    right_grid.draw();
#endif
    mosaic.draw(top::config.mosaic_x, top::config.mosaic_y, top::config.mosaic_width, top::config.mosaic_height);
    //   mosaic.draw();

  }

  /* ------------------------------------------------------------------------- */

  static void on_new_file(ImageCollector* col, const CollectedFile& file) {

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

    switch (file.type) {
      case COL_FILE_TYPE_NONE: {
        RX_ERROR("The collected file type isn't set!");
        break;
      }
      case COL_FILE_TYPE_RAW: {

        RX_VERBOSE("Got a raw file: %s", file.filename.c_str());
        if (0 != shop->mosaic.analyzeCPU(filepath)) {
          RX_ERROR("Failed to add a new file for the cpu analyzer. Check messages above");
        }
        break;
      }
      case COL_FILE_TYPE_LEFT_GRID: { 
        /* pass along; are processed in a separate thread */
        shop->img_processor.process(file);
        break;
      }
      case COL_FILE_TYPE_RIGHT_GRID: {
        /* pass along; are processed in a separate thread */
        shop->img_processor.process(file);
        break;
      }
      default: {
        RX_ERROR("Unhandled collected file type: %d", file.type);
        break;
      }
    }
  } 

  /* ------------------------------------------------------------------------- */
  
} /* namespace top */
