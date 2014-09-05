#include <mosaic/Config.h>
#include <mosaic/VideoInput.h>
#include <uv.h> /* TMP for uv_thread_self() */

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

    if (0 != mos::config.validateWindowSize()) {
      RX_ERROR("Cannot init VideoInput");
      return -3;
    }

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
      return -2;
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

    if (0 == is_init) {
      return; 
    }

#if !defined(NDEBUG)
    if (0 != mos::config.validateWindowSize()) {
      return;
    }
#endif

    needs_update = capture.needs_update;

    capture.update();

    /* update the input texture */
    if (needs_update) {
      glViewport(0, 0, mos::config.webcam_width, mos::config.webcam_height);
      fbo.bind();
        capture.draw();
      fbo.unbind();
      glViewport(0, 0, mos::config.window_width, mos::config.window_height);
    }
  }

  void VideoInput::draw() {

    if (0 == is_init) {
      return; 
    }

#if !defined(NDEBUG)
    if (0 != mos::config.validateWindowSize()) {
      return;
    }
#endif

    /* draw the webcam to screen (tiny) */
    capture.draw(0, 0, capture.width >> 2, capture.height >> 2);
    //capture.draw(0, 0, mos::config.window_width, mos::config.window_height);
  }


} /* namespace mos */

#else


/* --------------------------------------------------------------------------------- */
/*                            R T M P   I N P U T                                    */
/* --------------------------------------------------------------------------------- */

namespace mos {


  /* --------------------------------------------------------------------------------- */
  static void on_backup_player_video_frame(rxp::PlayerGL* gl, rxp_packet* pkt);
  static void on_video_frame(AVFrame* frame, void* user);
  static void on_player_event(vid::Player* player, int event);
  /* --------------------------------------------------------------------------------- */

  VideoInput::VideoInput() 
    :needs_update(false)
    ,is_init(false)
    ,restart_time(0)
    ,video_tex(0)
    ,state(MOS_VID_STATE_NONE)
  {
    int r = pthread_mutex_init(&mutex, NULL);
    if (r != 0) {
      RX_ERROR("Cannot initialize the mutex: %s", strerror(errno));
    }
  }

  VideoInput::~VideoInput() {
    needs_update = false;
    is_init = false;
    restart_time = 0;
    video_tex = 0;
    state = MOS_VID_STATE_NONE;

#if USE_BACKUP_PLAYER
    backup_player.on_event = NULL;
    backup_player.on_video_frame = NULL;
#endif

    int r = pthread_mutex_destroy(&mutex);
    if (r != 0) {
      RX_ERROR("Cannot destroy the mutex: %s", strerror(errno));
    }
  }

  int VideoInput::init() {

    int r = 0;

    if (true == is_init) {
      RX_ERROR("VideoInput already initialized, first call shutdown.");
      return -100;
    }

    if (0 != mos::config.validateStream()) {
      return -1;
    }

    backup_file = rx_to_data_path("video/backup.ogg");
    if (false == rx_file_exists(backup_file)) {
      RX_ERROR("Cannot find the backup file %s", backup_file.c_str());
      return -101;
    }
#if USE_BACKUP_PLAYER
    if (0 != backup_player.init(backup_file)) {
      RX_ERROR("Cannot initialize the backup player.");
      return -102;
    }

    if (0 != backup_player.play()) {
      RX_ERROR("Cannot start the backup player.");
      return -103;
    }

    backup_player.on_video_frame = on_backup_player_video_frame;
    backup_player.user = this;
#endif
    RX_VERBOSE("Initialize the YUV420P shader with a resolution of %d x %d.", mos::config.stream_width, mos::config.stream_height);

    if (0 != yuv.init(mos::config.stream_width, mos::config.stream_height)) {
      RX_ERROR("Cannot initialize the yuv decoder");
#if USE_BACKUP_PLAYER
      backup_player.shutdown();
#endif
      return -2;
    }

    /* we render the webcam into a FBO. */
    r =  fbo.init(mos::config.analyzer_width, mos::config.analyzer_height);
    if (r != 0) {
      RX_ERROR("Cannot initialize the FBO: %d", r);
      yuv.shutdown();
#if USE_BACKUP_PLAYER
      backup_player.shutdown();
#endif
      return -3;
    }

    video_tex = fbo.addTexture(GL_RGBA8, fbo.width, fbo.height, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0);
    if (0 != fbo.isComplete()) {
      RX_ERROR("FBO not complete");
      yuv.shutdown();
      fbo.shutdown();
#if USE_BACKUP_PLAYER
      backup_player.shutdown();
#endif
      return -4;
    }

    player.on_event = on_player_event;
    player.on_frame = on_video_frame;
    player.user = this;

    if (0 != player.init(config.stream_url)) {
      restart_time = time(NULL) + 5;
      RX_ERROR("RTMP player didn't want to connect, but we'll try to connect in a couple of seconds");
    }

    is_init = true;
    state = MOS_VID_STATE_CONNECTING;

    return 0;
  }

  void VideoInput::update() {

    if (false == is_init) {
      return;
    }

    if (0 == mos::config.window_width) {
      RX_ERROR("mos::config.window_width == 0... not good!");
      return;
    }

    /* lock the state and time. */
    uint64_t local_rtime;
    int local_state;
    lock();
      local_rtime = restart_time;
      local_state = state;  
    unlock();

    /* force restart when libav is not calling the sigpipe */
    if (1 != player.stream.is_stream_open && 0 == local_rtime) {
      lock();
        restart_time = time(NULL) + 1;
      unlock();
    }
    
    /* restart the player if necessary */
    if (0 != local_rtime) {
      uint64_t now = time(NULL);
      if (now > local_rtime) {
        RX_VERBOSE("Restarting rtmp stream: %s", mos::config.stream_url.c_str());
        player.init(mos::config.stream_url);
        lock();
          restart_time = 0;
        unlock();
      }
    }

    player.update();
   
    if (MOS_VID_STATE_PLAYING == local_state) {

      if (is_init && needs_update) {
        glViewport(0, 0, fbo.width, fbo.height);
        fbo.bind();
          yuv.draw();
        fbo.unbind();
        glViewport(0, 0, mos::config.window_width, mos::config.window_height);
      }

      /* shutdown the backup player if it's still running. */
#if USE_BACKUP_PLAYER
      if (0 == backup_player.isInit()) {
        backup_player.stop();
        backup_player.shutdown();
      }
#endif
    }
    else {

#if USE_BACKUP_PLAYER
      /* restart the backup player if necessary */
      if (0 != backup_player.isInit()) {
        RX_VERBOSE("START PLAYING");
        backup_player.init(backup_file);
        backup_player.play();
      }

      backup_player.update();
      
      glViewport(0, 0, fbo.width, fbo.height);
      fbo.bind();
         backup_player.draw();
      fbo.unbind();
      glViewport(0, 0, mos::config.window_width, mos::config.window_height);
#endif
    }
  }

  void VideoInput::draw() {
      int local_state;
      lock();
        local_state = state;
      unlock();

      glViewport(0, 0, yuv.w >> 2, yuv.h >> 2);
      {
        if (MOS_VID_STATE_PLAYING == local_state) {
          yuv.draw();
        }
        else {
#if USE_BACKUP_PLAYER
          backup_player.draw(0, 0, yuv.w >> 2, yuv.h >> 2);      
#endif
        }
      }
      glViewport(0, 0, mos::config.window_width, mos::config.window_height);
  }

  int VideoInput::shutdown() {
    int r = 0;

    if (false == is_init) {
      RX_ERROR("Trying to shutdown the video input but we're not initialized.");
      return -1;
    }

    if (0 != player.shutdown()) {
      RX_ERROR("Error while trying to shutdown the player.");
      r = -2;
    }

#if USE_BACKUP_PLAYER
    if (0 == backup_player.isPlaying()) {
#if 0
      if (0 != backup_player.stop()) {
        RX_ERROR("Failed to stop the backup player.");
      }
#endif
    }

    if (0 != backup_player.shutdown()) {
      RX_ERROR("Error while trying to shutdown the backup player.");
      r = -3;
    }
#endif

    needs_update = false;
    is_init = false;

    lock();
       restart_time = 0;
       state = MOS_VID_STATE_NONE;
    unlock();

    return r;
  }

  /* --------------------------------------------------------------------------------- */

  /* gets called by the backup player when it gets a new frame. */
  static void on_backup_player_video_frame(rxp::PlayerGL* gl, rxp_packet* pkt) {

    if (NULL == gl) {
      RX_ERROR("Invalid PlayerGL pointer.");
      return;
    }

    VideoInput* vid = static_cast<VideoInput*>(gl->user);
    if (NULL == vid) {
      RX_ERROR("Cannot cast to VideoInput");
      return;
    }

    vid->needs_update = true;
  }

  /* gets called by the video stream player when it gets a new frame. */
  static void on_video_frame(AVFrame* frame, void* user) {

    int r = 0;

    /* lots of error checking.. we have no control over the incoming video stream. */
    if (NULL == frame)      {  RX_ERROR("Received an NULL avframe.");  return;   } 
    if (0 == frame->width)  {  RX_ERROR("Invalid frame width");        return;   }
    if (0 == frame->height) {  RX_ERROR("Invalid frame height");       return;   }

    VideoInput* vid = static_cast<mos::VideoInput*>(user);
    if (NULL == vid) {
      RX_ERROR("Invalid vid ptr.");
      return ;
    }

    /* update yuv. */
    vid::YUV420P& yuv = vid->yuv;

    if (0 == yuv.w || 0 == yuv.h) {
      RX_ERROR("The yuv decoder has a invalid resolution: %d x %d", yuv.w, yuv.h);
      return;
    }

    if (frame->width != yuv.w) {
      RX_ERROR("The given frame width is not the same as the one we initialized the yuv decoder with: %d != %d || %d", frame->width, yuv.w, vid->fbo.width);
      return;
    }

    if (frame->height != yuv.h) { 
      RX_ERROR("The given frame height is not the same as the one we initialized the yuv decoder with: %d != %d || %d", frame->height, yuv.h, vid->fbo.height);
      return;
    }

    if (NULL == frame->data[0]) {
      RX_VERBOSE("Invalid data.");
      return;
    }

    yuv.update(frame->data[0], frame->linesize[0],
                frame->data[1], frame->linesize[1],
                frame->data[2], frame->linesize[2]);
    
    vid->needs_update = true;
  }

  static void on_player_event(vid::Player* player, int event) {

    VideoInput* vid = static_cast<VideoInput*>(player->user);
    if (NULL == vid) {
      RX_ERROR("Cannot get handle to the VideoInput");
      return;
    }

    if (VID_EVENT_SHUTDOWN == event || VID_EVENT_INIT_ERROR == event) {
      RX_VERBOSE("Received VID_EVENT_SHUTDOWN - restarting the stream.");
      vid->lock();
        vid->restart_time = time(NULL) + 5; /* start after a couple of seconds. */
        vid->state = MOS_VID_STATE_CONNECTING;
      vid->unlock();
    }
    else if (VID_EVENT_INIT_SUCCESS) {
      vid->lock();
        vid->state = MOS_VID_STATE_PLAYING;
      vid->unlock();   
    }
    else {
      RX_VERBOSE("Unhandled player event: %d", event);
    }
  }
  
} /* namespace mos */

#endif
