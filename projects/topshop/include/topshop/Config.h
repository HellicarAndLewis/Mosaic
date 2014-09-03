#ifndef TOPSHOP_CONFIG_H
#define TOPSHOP_CONFIG_H

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace top {

  int load_config();
  int save_config();

  class Config {
  public:
    Config();
    ~Config();
    void reset();
    int validate();  /* returns 0 on success else < 0 */

  public:
    int is_fullscreen;  /* 1 = yes, 0 = no */
    int window_width;
    int window_height;
    int mosaic_width;
    int mosaic_height;
    int mosaic_x;
    int mosaic_y;
    int grid_rows;
    int grid_cols;
    int grid_padding_x;
    int grid_padding_y;
    int grid_file_width;
    int grid_file_height;
    int left_grid_x;
    int left_grid_y;
    int right_grid_x;
    int right_grid_y;

    std::string raw_left_grid_filepath;    /* we watch for new files to pre-process in this directory for the left grid.*/
    std::string raw_right_grid_filepath;   /* we watch for new files to pre-process in this directory for the right grid. */
    std::string left_grid_filepath;        /* processed files for the left grid are moved to this directory. */
    std::string right_grid_filepath;       /* processed files for the right grid are moved to this directory. */
  };

  extern Config config;

} /* namesapce top */

#endif
