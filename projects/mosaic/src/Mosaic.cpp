#include <mosaic/Mosaic.h>

namespace mos {

  Mosaic::Mosaic() {
  }

  Mosaic::~Mosaic() {
    shutdown();
  }

  int Mosaic::init() {
    int r;

    /* initialize the video input. */
    r = video_input.init();
    if (0 != r) {
      return r;
    }
    
    /* initialize the feature extractor and comparator. */
    r = featurex.init(video_input.texid());
    if (0 != r) {
      return r;
    }

    return 0;
  }

  void Mosaic::update() {
    video_input.update();
    
    /* new input, update gpu analyzer. */
    if (video_input.needsUpdate()) {
      featurex.analyzeGPU();
    }
  }

  void Mosaic::draw() {
    video_input.draw();
    featurex.draw();
  }

  int Mosaic::shutdown() {
    int r;

    r = video_input.shutdown();
    if (0 != r) {
      return r;
    }

    r = featurex.shutdown();
    if (0 != r) {
      return r;
    }

    return 0;
  }

} /* namespace mos */
