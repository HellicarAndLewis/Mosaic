#include <mosaic/Config.h>

namespace mos {


  /* --------------------------------------------------------------------------------- */

  Config config;

  /* --------------------------------------------------------------------------------- */
  
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

    return validateAnalyzerSize();
  }
  
  int Config::validateAnalyzerSize() {
    if (0 == analyzer_width) {
      RX_ERROR("Invalid analyzer width.");
      return -1;
    }
    if (0 == analyzer_height) {
      RX_ERROR("Invalid analyzer height.");
      return -2;
    }
    return 0;
  }

  int Config::validateWindowSize() {
    if (0 == mos::config.window_width) {
      RX_ERROR("Invalid window width.");
      return -1;
    }
    if (0 == mos::config.window_height) {
      RX_ERROR("Invalid window height.");
      return -2;
    }
    return 0;
  }

  int Config::validateStream() {
    if (0 == stream_width) {
      RX_ERROR("Invalid stream width.");
      return -1;
    }
    if (0 == stream_height) {
      RX_ERROR("Invalid stream height");
      return -2;
    }
    if (0 == stream_url.size()) {
      RX_ERROR("Invalid stream url");
      return -3;
    }

    return validateAnalyzerSize();
  }

  void Config::reset() {
    analyzer_width = 0;
    analyzer_height = 0;
    webcam_device = 0;
    webcam_width = 0;
    webcam_height = 0;
    window_width = 0;
    window_height = 0;
    stream_width = 0;
    stream_height = 0;
    stream_url.clear();
  }
  
} /* namespace mos */
