#include <grid/SimpleSettings.h>

namespace grid {

  SimpleSettings::SimpleSettings() 
    :img_width(0)
    ,img_height(0)
    ,cols(0)
    ,rows(0)
    ,padding_x(0)
    ,padding_y(0)
    ,offset_x(0)
    ,offset_y(0)
    ,direction(SIMPLE_GRID_DIRECTION_NONE)
  {
  }

  SimpleSettings::~SimpleSettings() {
    img_width = 0;
    img_height = 0;
    cols = 0;
    rows = 0;
    padding_x = 0;
    padding_y = 0;
    offset_x = 0;
    offset_y = 0;
    direction = SIMPLE_GRID_DIRECTION_NONE;
    watch_dir.clear();
    image_dir.clear();
  }

  bool SimpleSettings::validate() {
    if (0 == img_width) {  RX_ERROR("Invalid width"); return false;  }
    if (0 == img_height) {  RX_ERROR("Invalid height"); return false;  }
    if (0 == cols) { RX_ERROR("Invalid cols"); return false; } 
    if (0 == rows) { RX_ERROR("Invalid rows"); return false; } 
    if (SIMPLE_GRID_DIRECTION_LEFT != direction && SIMPLE_GRID_DIRECTION_RIGHT != direction ) { RX_ERROR("No direction set"); return false; }

    if (false == rx_is_dir(watch_dir)) {
      RX_ERROR("Not a directory: %s", watch_dir.c_str());
      return false;
    }

    if (false == rx_is_dir(image_dir)) {
      RX_ERROR("Not a directory: %s", image_dir.c_str());
      return false;
    }

    return true;
  }

} /* namespace grid */
