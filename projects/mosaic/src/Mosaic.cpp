#include <mosaic/Mosaic.h>

namespace mos {

  /* TESTING */
  static void on_pixdata(int x, int y, int w, int h, unsigned char* pixels, void* user);
  /* END TESTING */

  Mosaic::Mosaic()
    :mosaic_tex(0)
  {
  }

  Mosaic::~Mosaic() {
    shutdown();
  }

  int Mosaic::init() {
    int r;

    /* TESTING */
    featurex.on_pixdata = on_pixdata;
    featurex.user = this;
    /* END TESTING */

    if (0 != mos::load_config()) {
      RX_ERROR("Cannot load config. stopping.");
      return -99;
    }

    /* get the texture sizes for the mosaic. */
    int mw = fex::config.getMosaicImageWidth();
    int mh = fex::config.getMosaicImageHeight();
    if (0 > mw) {
      return -100;
    }
    if (0 > mh) {
      return -101;
    }

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

    /* create the mosaic texture */
    glGenTextures(1, &mosaic_tex);
    glBindTexture(GL_TEXTURE_2D, mosaic_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mw, mh, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* initialize the async uploader */
    if (0 != async_upload.init(mw, mh, GL_RGBA)) {
      return -102;
    }

    return 0;
  }

  void Mosaic::update() {

    /* update the input texture - is used by the gpu analyzer.*/
    video_input.update();
    
    /* new input, update gpu analyzer. */
    if (video_input.needsUpdate()) {
      featurex.analyzeGPU();
      featurex.match();
      
      if (NULL != featurex.mosaic_pixels) {
        glBindTexture(GL_TEXTURE_2D, mosaic_tex);
        async_upload.upload(featurex.mosaic_pixels);
      }
    }
  }

  void Mosaic::draw() {

    /* draw the result. */
    painter.clear();
    painter.texture(mosaic_tex, 0, mos::config.window_height, mos::config.window_width, -mos::config.window_height);
    painter.draw();

    /* draw a debug image. */
    video_input.draw();
  }

  int Mosaic::shutdown() {
    int r = 0;
    int result = 0;

    if (0 != mosaic_tex) {
      glDeleteTextures(1, &mosaic_tex);
    }

    r = async_upload.shutdown();
    if (r != 0) {
      RX_WARNING("Error while trying to shutdown the async uploader.");
      result = -1;
    }

    r = video_input.shutdown();
    if (0 != r) {
      RX_WARNING("Error while trying to shutdown the video input");
      result = -2;
    }

    r = featurex.shutdown();
    if (0 != r) {
      RX_WARNING("Error while trying to shutdown the feature extractor");
      result = -3;
    }

    return result;
  }

  /* TESTING */
  static void on_pixdata(int x, int y, int w, int h, unsigned char* pixels, void* user) {
    Mosaic* m = static_cast<Mosaic*>(user);
    if (NULL == m) {
      RX_ERROR("mosaic pointer invalid.");
      return;
    }
    int mw = fex::config.getMosaicImageWidth();
    int mh = fex::config.getMosaicImageHeight();
    glBindTexture(GL_TEXTURE_2D, m->mosaic_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
  }
  /* END TESTING */

} /* namespace mos */
