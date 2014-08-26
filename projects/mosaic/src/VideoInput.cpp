#include <mosaic/Config.h>
#include <mosaic/VideoInput.h>

#if USE_WEBCAM_AS_INPUT
/* --------------------------------------------------------------------------------- */
/*                             W E B C A M   I N P U T                               */
/* --------------------------------------------------------------------------------- */
namespace mos {

  VideoInput::VideoInput() 
    :is_init(0)
  {
  }

  VideoInput::~VideoInput() {
    if (1 == is_init) {
      shutdown();
    }
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
      return r;
    }

    r = capture.start();
    if (0 >= r) {
      RX_ERROR("Error while starting the webcam: %d", r) ;
      return r;
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

    is_init = 0;

    return 0;
  }

  void VideoInput::update() {
    if (0 == is_init) {  return;  }
    capture.update();
  }

  void VideoInput::draw() {
    if (0 == is_init) { return; }
    capture.draw();
  }


} /* namespace mos */

#else
/* --------------------------------------------------------------------------------- */
/*                            R T M P   I N P U T                                    */
/* --------------------------------------------------------------------------------- */
#error "Need to implement RTMP input for VideoInput"
#endif
