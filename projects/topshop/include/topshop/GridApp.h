/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

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
