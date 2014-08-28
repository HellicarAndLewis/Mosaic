#include <featurex/Config.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace fex {
  
  Config config;

  Config::Config() 
    :input_tile_size(0)
    ,input_image_width(0)
    ,input_image_height(0)
    ,cols(0)
    ,rows(0)
    ,show_timer(0)
    ,file_tile_width(0)
    ,file_tile_height(0)
    ,memory_pool_size(0)
  {
  }

  Config::~Config() {
    input_tile_size = 0;
    input_image_width = 0;
    input_image_height = 0;
    cols = 0;
    rows = 0;
    show_timer = 0;
    file_tile_width = 0;
    file_tile_height = 0;
    memory_pool_size = 0;
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

    if (0 == input_tile_size) {
      RX_ERROR("fex::config.input_tile_size == 0. invalid");
      return false;
    }

    return true;
  }

  bool Config::validateTilePoolSettings() {

    if (0 == file_tile_width) {
      RX_ERROR("fex::config.file_tile_width = 0. invalid");
      return false;
    }

    if (0 == file_tile_height) {
      RX_ERROR("fex::config.file_tile_height = 0. invalid");
      return false;
    }

    if (0 == memory_pool_size) {
      RX_ERROR("fex::config.memory_pool_size = 0. invalid");
      return false;
    }

    uint64_t megs = 1 + ((uint64_t)file_tile_width * (uint64_t)file_tile_height * (uint64_t)memory_pool_size * 4llu) / (1024llu * 1024llu);
    //RX_VERBOSE("The tile pool size needs %llu megabytes of ram", megs);

    /* just a safety check so we're not doing stuff w/o testing. */
    if (4000 < megs) {
      RX_ERROR("It seems that you want to allocate more then 4000 megabytes; this is probably doable; but didn't test performance with this amount.");
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

  uint64_t Config::getTilePoolSizeInBytes() {
    uint64_t nbytes = ((uint64_t)file_tile_width * (uint64_t)file_tile_height * (uint64_t)memory_pool_size * 4llu);
    return nbytes;
  }

  int Config::getMosaicImageWidth() {
    if (false == validateTileSettings()) {
      return -1;
    }
    if (false == validateTilePoolSettings()) {
      return -2;
    }
    return (cols * file_tile_width);
  }

  int Config::getMosaicImageHeight() {
    if (false == validateTileSettings()) {
      return -1;
    }
    if (false == validateTilePoolSettings()) {
      return -2;
    }
    return (rows * file_tile_height);
  }


} /* namespace fex */
