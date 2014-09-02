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
//#  error "Need to implement the RTMP stream input"  
#include <glad/glad.h>
#include <gfx/FBO.h>
#include <video/YUV420P.h>
#include <video/Player.h>
#include <sys/time.h>
#include <rxp_player/PlayerGL.h>
#include <tinylib.h>
#include <stdio.h>
#include <pthread.h>

#define MOS_VID_STATE_NONE 0x00
#define MOS_VID_STATE_CONNECTING 0x01
#define MOS_VID_STATE_PLAYING 0x02

#define USE_BACKUP_PLAYER 1

namespace mos {

  class VideoInput {
  public:
    VideoInput();
    ~VideoInput();
    int init();
    void update();
    void draw();
    int shutdown();
    GLuint texid();
    int needsUpdate();                          /* checks if we updated a video frame; will reset the internal flag once called. */
    
    void lock();
    void unlock();
    
  public:
    int state;                                  /* used to keep track of the current state; and based on the state we show either a pre-recorded video or the live stream */
    bool is_init;
    bool needs_update;
    uint64_t restart_time;
    GLuint video_tex;
    std::string backup_file;
    vid::Player player;
#if USE_BACKUP_PLAYER
    rxp::PlayerGL backup_player;                /* when we get disconnection from the remote stream we use the backup player instead. */
#endif
    vid::YUV420P yuv;
    gfx::FBO fbo;                               /* we write the decoded video to a FBO, RTT, and the texture is used by the GPU analyzer. */
    pthread_mutex_t mutex;
  };
  
  inline GLuint VideoInput::texid() {
    return video_tex;
  }

  inline int VideoInput::needsUpdate() {
    bool needs = false;
    needs = needs_update;
    needs_update = false; 
    return (needs) ? 0 : -1;
  }

  inline void VideoInput::lock() {
    int r = pthread_mutex_lock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed when trying to lock: %s", strerror(r));
    }
  }

  inline void VideoInput::unlock() {
    int r = pthread_mutex_unlock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed when trying to unlock: %s", strerror(r));
    }
  }


} /* namespace mos */


#endif /* #if USE_WEBCAM_AS_INPUT */

#endif /* ROXLU_MOSAIC_VIDEO_INPUT_H */

