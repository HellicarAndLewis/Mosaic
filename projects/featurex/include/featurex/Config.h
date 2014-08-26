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

namespace fex {

struct Config {
  
  Config();
  ~Config();
  bool validateTileSettings();              /* validate the mosaic tile image settings */
  bool validateAnalyzerSettings();          /* validate the settings for the offline analyzer */
  bool validateTilePoolSettings();          /* validate the tile pool settings */

  uint64_t getTilePoolSizeInBytes();        /* returns how many bytes you would need for the tile pool */

  /* tile settings */
  int input_tile_size;
  int input_image_width;
  int input_image_height;
  int cols;
  int rows;
  bool show_timer;

  /* tile memory pool */
  int tile_width;                         /* the width of the tiles that are rendered into the mosaic. */
  int tile_height;                        /* the height of the tiles that are rendered into the mosaic. */
  int tile_pool_size;                     /* number of tiles that we can store in RAM, we allocate a huge buffer in TilePool for this amount of images, with a size of (tile_width * tile_height) */

  /* analyzer settings */
  std::string raw_filepath;               /* where downloaded, raw instagram images will be stored */
  std::string resized_filepath;           /* will contain the resized versions of the input images */
  std::string blurred_filepath;           /* will contains the blurred version of the input images */
};

extern Config config;

} /* namespace fex */

#endif
