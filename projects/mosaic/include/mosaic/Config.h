#ifndef MOSAIC_CONFIG_H
#define MOSAIC_CONFIG_H

#define USE_WEBCAM_AS_INPUT 0         /* when set to 1 we will use a webcam feed as input for the mosaic generator. */

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
    int validateStream();              /* checks stream_width, stream_height, stream_url */
    int validateAnalyzerSize();        /* checks if the analyzer width and height have been set. */

  public:
    int analyzer_width;                /* either the webcam or rtmp-stream will be resized to this width, then the resulting image is used by the gpu analyzer. */
    int analyzer_height;               /* either the webcam or rtmp-stream will be resized to this height, then the resulting image is used by the gpu analyzer. */
    int webcam_device;                 /* when USE_WEBCAM_AS_INPUT has been set to 1, this will be used by the VideoInput class */
    int webcam_width;
    int webcam_height;
    int window_width;                  /* width of the window; can be used with glViewport for example */
    int window_height;                 /* the height of the window. */
    int stream_width;                  /* width of the rtmp video stream, necessary so we can setup the decoder textures */
    int stream_height;                 /* height of the rtmp video stream, necessary so we can setup the decoder textures */
    std::string stream_url;            /* the url of the rtmp video stream. */
    
  };

  /* --------------------------------------------------------------------------------- */

  extern Config config;
  
} /* namespace mos */

#endif
