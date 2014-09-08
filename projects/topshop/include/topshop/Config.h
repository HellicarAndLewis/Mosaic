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
    int is_fullscreen;                       /* 1 = yes, 0 = no */
    int is_debug_draw;                       /* 1 = yes, 0 = no */
    int grid_left_monitor;
    int grid_right_monitor;
    int mosaic_monitor;
    int log_level;                           /* any of the RX_LOG_LEVEL values, 0-4 */
    int mosaic_win_width;
    int mosaic_win_height;
    int mosaic_width;
    int mosaic_height;
    int mosaic_x;
    int mosaic_y;
    int grid_win_width;
    int grid_win_height;
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

    std::string remote_state_url;          /* we fetch an JSON state object every X seconds which changes the behavior of the application; at the time of writing it's only used to toggle between video and mosaic */
    std::string polaroid_filepath;         /* where we store the big polaroid version of the image */
    std::string json_filepath;             /* the path where the meta data for the image files is stored. */
    std::string raw_left_grid_filepath;    /* we watch for new files to pre-process in this directory for the left grid.*/
    std::string raw_right_grid_filepath;   /* we watch for new files to pre-process in this directory for the right grid. */
    std::string left_grid_filepath;        /* processed files for the left grid are moved to this directory. */
    std::string right_grid_filepath;       /* processed files for the right grid are moved to this directory. */
  };

  extern Config config;

} /* namesapce top */

#endif
