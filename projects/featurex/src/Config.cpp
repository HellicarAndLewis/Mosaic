#include <featurex/Config.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace fex {
  
  Config config;

  Config::Config() 
    :tile_size(0)
    ,input_image_width(0)
    ,input_image_height(0)
    ,cols(0)
    ,rows(0)
    ,show_timer(0)
  {
  }

  Config::~Config() {
    tile_size = 0;
    input_image_width = 0;
    input_image_height = 0;
    cols = 0;
    rows = 0;
    show_timer= 0;
  }

  bool Config::validateTileSettings() {

    if (0 == cols) {
      RX_ERROR("fex::config.cols == 0. invalid");
      return false;
    }

    if (0 == rows) {
      RX_ERROR("fex::config.rows == 0. invalid");
      return false;
    }

    if (0 == input_image_width) { 
      RX_ERROR("fex::config.input_image_width == 0. invalid");
      return false;
    }

    if (0 == input_image_height) {
      RX_ERROR("fex::config.input_image_height == 0. invalid");
      return false;
    }

    if (0 == tile_size) {
      RX_ERROR("fex::config.tile_size == 0. invalid");
      return false;
    }

    return true;
  }

  bool Config::validateAnalyzerSettings() {

    if (0 == raw_filepath.size()) {
      RX_ERROR("fex::config.raw_filepath not set");
      return false;
    }

    if (0 == resized_filepath.size()) {
      RX_ERROR("fex::config.resized_filepath not set");
      return false;
    }

    if (0 == blurred_filepath.size()) {
      RX_ERROR("fex::config.blurred_filepath not set");
      return false;
    }

    return true;
  }

} /* namespace fex */
