#include <mosaic/Config.h>

namespace mos {

  Config config;
  
  Config::Config() {
    reset();
  }

  Config::~Config() {
    reset();
  }

  
  int Config::validateWebcam() {
    if (0 == webcam_width) {
      RX_ERROR("Invalid width");
      return -1;
    }
    if (0 == webcam_height) {
      RX_ERROR("Invalid height");
      return -1;
    }
    return 0;
  }

  void Config::reset() {
    webcam_device = 0;
    webcam_width = 0;
    webcam_height = 0;
  }
  
} /* namespace mos */
