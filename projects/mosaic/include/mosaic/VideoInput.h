#ifndef ROXLU_MOSAIC_VIDEO_INPUT_H
#define ROXLU_MOSAIC_VIDEO_INPUT_H

#include <mosaic/Config.h>

#if USE_WEBCAM_AS_INPUT
/* --------------------------------------------------------------------------------- */
/*                             W E B C A M   I N P U T                               */
/* --------------------------------------------------------------------------------- */
#include <glad/glad.h>
#include <videocapture/CaptureGL.h>
#include <gfx/FBO.h>

namespace mos {

  class VideoInput {

  public:
    VideoInput();
    ~VideoInput();
    int init();
    void update();
    void draw();
    int shutdown();
     
  public:
    ca::CaptureGL capture;
    int is_init;
  };

} /* namespace mos */

#else /* #if USE_WEBCAM_AS_INPUT */

/* --------------------------------------------------------------------------------- */
/*                            R T M P   I N P U T                                    */
/* --------------------------------------------------------------------------------- */
#  error "Need to implement the RTMP stream input"  


#endif /* #if USE_WEBCAM_AS_INPUT */

#endif /* ROXLU_MOSAIC_VIDEO_INPUT_H */
