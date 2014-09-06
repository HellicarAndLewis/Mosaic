#include <topshop/Config.h>
#include <featurex/Config.h>
#include <mosaic/Config.h>
#include <tracking/InteractiveGrid.h>

namespace track {

  /* ---------------------------------------------------------------------------------- */

  InteractiveCell::InteractiveCell() 
    :col(0)
    ,row(0)
    ,timeout(0)
  {
  }

  InteractiveCell::~InteractiveCell() {
    col = 0;
    row = 0;
    timeout = 0;
  }

  /* ---------------------------------------------------------------------------------- */

  InteractiveGrid::InteractiveGrid()
    :tracker(NULL)
    ,tiles(NULL)
    ,on_activate(NULL)
    ,user(NULL)
  {
  }

  InteractiveGrid::~InteractiveGrid() {
    cells.clear();
    user = NULL;
    on_activate = NULL;
    tracker = NULL;
    tiles = NULL;
  }

  int InteractiveGrid::init(Tracker* track, Tiles* til) {

    tracker = track;
    tiles = til;

    if (NULL == tracker) {
      RX_ERROR("Tracker ptr == NULL");
      return -1;
    }
    if (NULL == tiles) {
      RX_ERROR("Tiles ptr == NULL");
      return -2;
    }
    if (0 == mos::config.webcam_width) {
      RX_ERROR("Invalid webcam width: %d", mos::config.webcam_width);
      return -3;
    }
    if (0 == mos::config.webcam_height) {
      RX_ERROR("Invalid webcam width: %d", mos::config.webcam_height);
      return -4;
    }
    if (0 == fex::config.rows) {
      RX_ERROR("Invalid grid_rows");
      return -5;
    }
    if (0 == fex::config.cols) {
      RX_ERROR("Invalid grid_cols");
      return -6;
    }

    /* setup cells. */
    cells.resize(fex::config.cols * fex::config.rows);
    for (int i = 0; i < fex::config.cols; ++i) {
      for (int j = 0; j < fex::config.rows; ++j) {
        int dx = j * fex::config.cols + i;
        cells[dx].col = i;
        cells[dx].row = j;
      }
    }
    
    return 0;
  }

  int InteractiveGrid::shutdown() {
    cells.clear();
    return 0;
  }

  /* This is where we detect blobs that are active for some time (see tail.size() check).
     When a blob is active for some time, we try to active the cell. But we also have to 
     check if the cell wasn't already triggered the previous frame. When its trigged
     we should show the correct image */
     
  void InteractiveGrid::update() {

    if (NULL == tracker) {
      RX_ERROR("Tracker is invalid.");
      return;
    }
    if (NULL == on_activate) {
      RX_ERROR("No on_activate set so no use to execute the update function.");
      return;
    }

    uint64_t now = rx_hrtime();
    float iv_w = 1.0f / mos::config.webcam_width;
    float iv_h = 1.0f / mos::config.webcam_height;

    /* get the detected blobs. */
    for (size_t i = 0; i < tracker->blobs.blobs.size(); ++i) {

      Blob& blob = tracker->blobs.blobs[i];
      if (false == blob.matched) {
        continue;
      }
      
      if (10 > blob.trail.size()) {
        continue;
      }
      
      /* convert the position of the blob to a cell index. */
      cv::Point& pt = blob.position;
      float px = float(pt.x) * iv_w;
      float py = float(pt.y) * iv_h;
      
      int col = px * fex::config.cols;
      int row = py * fex::config.rows;
      int dx = row * fex::config.cols + col;

      if (dx >= cells.size()) {
        RX_ERROR("Not supposed to happen, but the calculated index is bigger then the total number of cells, col: %d, row: %d, dx: %d", col, row, dx);
        continue;
      }

      InteractiveCell& cell = cells[dx];
      if (0 == cell.timeout || (0 != cell.timeout && now > cell.timeout)) {
        /* new cell, make active. */
        //RX_VERBOSE("Activated: %d x %d, timeout: %llu", col, row, cell.timeout);
        on_activate(col, row, user);
        cell.timeout = now + 3e9; /* this cell can be reused after X seconds (Xe9) */
      }
    }
  }


} /* namespace track */
