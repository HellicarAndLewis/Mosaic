#include <mosaic/Mosaic.h>

namespace mos {

  Mosaic::Mosaic() {
  }

  Mosaic::~Mosaic() {
    shutdown();
  }

  int Mosaic::init() {
    int r;

    /* Initialize the video input. */
    r = video_input.init();
    if (0 != r) {
      return r;
    }
    
    return 0;
  }

  void Mosaic::update() {
    video_input.update();
  }

  void Mosaic::draw() {
    video_input.draw();
  }

  int Mosaic::shutdown() {
    int r;

    /* shutdown video input (RTMP or webcam) */
    r = video_input.shutdown();
    if (0 != r) {
      return r;
    }

    return 0;
  }

} /* namespace mos */
