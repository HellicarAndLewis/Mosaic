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
    GLuint texid();                  /* returns the texture id into which we write the webcam images. */
    int needsUpdate();               /* returns 0 when we need to update the mosaic otherwise -1 */
     
  public:
    ca::CaptureGL capture;
    gfx::FBO fbo;                    /* we write the decoded video to a FBO, RTT, and the texture is used by the GPU analyzer. */
    GLuint webcam_tex;
    int is_init;
    bool needs_update;               /* is set to true when the webcam has a new image */ 
  };

  inline GLuint VideoInput::texid() {
    return webcam_tex;
  }

  inline int VideoInput::needsUpdate() {
    return (needs_update) ? 0 : -1;
  }

} /* namespace mos */

#else /* #if USE_WEBCAM_AS_INPUT */

/* --------------------------------------------------------------------------------- */
/*                            R T M P   I N P U T                                    */
/* --------------------------------------------------------------------------------- */
#  error "Need to implement the RTMP stream input"  


#endif /* #if USE_WEBCAM_AS_INPUT */

#endif /* ROXLU_MOSAIC_VIDEO_INPUT_H */
