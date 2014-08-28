#ifndef MOSAIC_CONFIG_H
#define MOSAIC_CONFIG_H

#define USE_WEBCAM_AS_INPUT 1         /* when set to 1 we will use a webcam feed as input for the mosaic generator. */

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace mos {

  /* --------------------------------------------------------------------------------- */

  int load_config();
  int save_config();

  class Config {
  public:
    Config();
    ~Config();
    void reset();
    int validateWebcam();              /* validate the webcam settings; only applicable with USE_WEBCAM_AS_INPUT has been set to 1 */
    int validateWindowSize();          /* checks window_width, window_height; returns 0 on success else < 0 */

  public:
    int webcam_device;                 /* when USE_WEBCAM_AS_INPUT has been set to 1, this will be used by the VideoInput class */
    int webcam_width;
    int webcam_height;
    int window_width;                  /* width of the window; can be used with glViewport for example */
    int window_height;                 /* the height of the window. */
  };

  /* --------------------------------------------------------------------------------- */

  extern Config config;
  
} /* namespace mos */

#endif
