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

#ifndef FEATUREX_CONFIG_H
#define FEATUREX_CONFIG_H

#include <string>

/*
#define TILE_SIZE 32
#define INPUT_IMAGE_WIDTH 384
#define INPUT_IMAGE_HEIGHT 384
#define COLS (INPUT_IMAGE_WIDTH / TILE_SIZE)
#define ROWS (INPUT_IMAGE_HEIGHT / TILE_SIZE)
*/

namespace fex {

struct Config {
  
  Config();
  ~Config();
  bool validateTileSettings();              /* validate the mosaic tile image settings */
  bool validateAnalyzerSettings();          /* validate the settings for the offline analyzer */

  /* tile settings */
  int tile_size;
  int input_image_width;
  int input_image_height;
  int cols;
  int rows;
  bool show_timer;

  /* analyzer settings */
  std::string raw_filepath;    /* where downloaded, raw instagram images will be stored */
  std::string resized_filepath;  /* will contain the resized versions of the input images */
  std::string blurred_filepath;  /* will contains the blurred version of the input images */
};

extern Config config;

} /* namespace fex */

#endif
