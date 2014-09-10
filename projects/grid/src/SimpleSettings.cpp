#include <grid/SimpleSettings.h>


namespace grid {

  SimpleSettings::SimpleSettings() 
    :img_width(0)
    ,img_height(0)
    ,cols(0)
    ,rows(0)
  {
  }

  SimpleSettings::~SimpleSettings() {
    img_width = 0;
    img_height = 0;
    cols = 0;
    rows = 0;
    padding_x = 0;
    padding_y = 0;
  }

  bool SimpleSettings::validate() {
    if (0 == img_width) {  RX_ERROR("Invalid width"); return false;  }
    if (0 == img_height) {  RX_ERROR("Invalid height"); return false;  }
    if (0 == cols) { RX_ERROR("Invalid cols"); return false; } 
    if (0 == rows) { RX_ERROR("Invalid rows"); return false; } 
    return true;
  }

} /* namespace grid */
