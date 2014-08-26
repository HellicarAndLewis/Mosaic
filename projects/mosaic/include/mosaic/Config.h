#ifndef MOSAIC_CONFIG_H
#define MOSAIC_CONFIG_H

#define USE_WEBCAM_AS_INPUT 1         /* when set to 1 we will use a webcam feed as input for the mosaic generator. */

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace mos {

  /* --------------------------------------------------------------------------------- */

  class Config {
  public:
    Config();
    ~Config();
    void reset();
    int validateWebcam();              /* validate the webcam settings; only applicable with USE_WEBCAM_AS_INPUT has been set to 1 */

  public:
    int webcam_device;                 /* when USE_WEBCAM_AS_INPUT has been set to 1, this will be used by the VideoInput class */
    int webcam_width;
    int webcam_height;
  };

  /* --------------------------------------------------------------------------------- */

  extern Config config;
  
} /* namespace mos */

#endif
