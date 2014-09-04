#ifndef TOPSHOP_GRID_APP_H
#define TOPSHOP_GRID_APP_H

#include <mosaic/Config.h>
#include <topshop/Config.h>
#include <topshop/ImageCollector.h>
#include <topshop/ImageProcessor.h>
#include <grid/Grid.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace top {

  /* ------------------------------------------------------------------------- */

  struct GridAppSettings {
    GridAppSettings();
    ~GridAppSettings();
    void reset();
    int validate(); /* 0 = good, < 0 = error */

    std::string image_path;
    std::string watch_path;
    int image_width;
    int image_height;
    int grid_rows;
    int grid_cols;
  };

  /* ------------------------------------------------------------------------- */

  class GridApp {
  public:
    GridApp(int dir);
    ~GridApp();

    int init(GridAppSettings& cfg);
    int shutdown();
    void update();
    void draw();

  public:
    GridAppSettings settings;
    grid::Grid grid;
    top::ImageCollector img_collector;
    top::ImageProcessor img_processor;
  };
} /* namespace top */

#endif
