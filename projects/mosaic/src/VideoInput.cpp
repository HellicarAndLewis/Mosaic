#include <mosaic/Config.h>
#include <mosaic/VideoInput.h>

#if USE_WEBCAM_AS_INPUT
/* --------------------------------------------------------------------------------- */
/*                             W E B C A M   I N P U T                               */
/* --------------------------------------------------------------------------------- */
namespace mos {

  VideoInput::VideoInput() 
    :is_init(0)
    ,webcam_tex(0)
    ,needs_update(false)
  {
  }

  VideoInput::~VideoInput() {
    if (1 == is_init) {
      shutdown();
    }

    needs_update = false;
  }

  int VideoInput::init() {
    int r;

    /* only init once. */
    if (1 == is_init) {
      RX_ERROR("Trying to initialize the VideoInput, but we're already initialzed.");
      return -1;
    }

    /* validate webcam config */
    r = mos::config.validateWebcam();
    if (0 != r) {
      return r;
    }

    /* find the device. */
    int device_index = -1;
    std::vector<ca::Device> devices = capture.getDevices();
    for (size_t i = 0; i < devices.size(); ++i) {
      if (devices[i].index == mos::config.webcam_device) {
        device_index = devices[i].index;
      }
    }

    if (-1 == device_index) {
      RX_ERROR("The device index from mos::Config (%d) is not found. Did you set the correct device id?", mos::config.webcam_device);
      capture.listDevices();
      return -1;
    }

    /* initialze the webcam. */
    r = capture.open(device_index, mos::config.webcam_width, mos::config.webcam_height);
    if (0 != r) {
      RX_ERROR("Error while opening webcam device: %d, err: %d", device_index, r);
      capture.listDevices();
      return r;
    }

    r = capture.start();
    if (0 >= r) {
      RX_ERROR("Error while starting the webcam: %d", r) ;
      return r;
    }

    /* we render the webcam into a FBO. */
    r = fbo.init(mos::config.webcam_width, mos::config.webcam_height);
    if (r != 0) {
      RX_ERROR("Cannot initialize the FBO: %d", r);
      capture.stop();
      capture.close();
      return r;
    }

    webcam_tex = fbo.addTexture(GL_RGBA8, mos::config.webcam_width, mos::config.webcam_height, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
    if (0 != fbo.isComplete()) {
      RX_ERROR("FBO not complete");
      capture.stop();
      capture.close();
      return -200;
    }
    
    is_init = 1;

    return 0;
  }

  int VideoInput::shutdown() {

    int r;

    if (0 == is_init) {
      RX_WARNING("Not initialized, or already shutdown.");
      return -1;
    }

    r = capture.stop();
    if (0 >= r) {
      RX_ERROR("Error while stopping the video input: %d", r);
      /* we do ignore this on purpose */
    }

    r = capture.close();
    if (0 >= r) {
      RX_ERROR("Error while stopping the video input: %d", r);
    }

    r = fbo.shutdown();
    if (r != 0) {
      RX_ERROR("Error while cleaning up the FBO: %d", r);
    }

    is_init = 0;

    return 0;
  }

  void VideoInput::update() {
    if (0 == is_init) {  return;  }

    needs_update = capture.needs_update;

    capture.update();
  }

  void VideoInput::draw() {
    if (0 == is_init) { return; }

    if (needs_update) {
      fbo.bind();
        capture.draw();
      fbo.unbind();
    }

    /* draw the webcam to screen */
    /* capture.draw(); */
    capture.draw(0, 0, capture.width >> 2, capture.height >> 2);
  }


} /* namespace mos */

#else
/* --------------------------------------------------------------------------------- */
/*                            R T M P   I N P U T                                    */
/* --------------------------------------------------------------------------------- */
#error "Need to implement RTMP input for VideoInput"
#endif
