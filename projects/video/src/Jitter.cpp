#include <video/Jitter.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace vid {

  Jitter::Jitter()
    :first_pts(0)
     /* @todo - increase the jitter buffer time, we're just testing now ...  */
     ,buffer_size_ns(1000llu * 1000llu * 1000llu * 3)  /* we need at least 3 seconds before we start player. */
     //,buffer_size_ns(1000llu * 1000llu * 500llu)  /* we need at least 3 seconds before we start player. */
    ,curr_buffer_ns(0)
    ,time_base(0)
    ,curr_pts(0)
    ,start_pts(0)
    ,on_frame(NULL)
    ,on_event(NULL)
    ,user(NULL)
    ,state(JITTER_STATE_NONE)
  {
  }
  
  Jitter::~Jitter() {

    on_frame = NULL;
    on_event = NULL;
    user = NULL;
    
    shutdown();
  }

  int Jitter::init(double timebase) {

    if (timebase <= 0) {
      RX_ERROR("Invalid timebase");
      return -1;
    }

    time_base = timebase;

    return 0;
  }

  int Jitter::shutdown() {

    RX_VERBOSE("Cleaning up jitter.");

    /* free all frames that are buffered. */
    for (size_t i = 0; i < frames.size(); ++i) {
      av_frame_free(&frames[i]);
    }
    frames.clear();

    first_pts = 0;
    curr_buffer_ns = 0;
    time_base = 0;
    curr_pts = 0;
    start_pts = 0;
    state = JITTER_STATE_NONE;

    return 0;
  }

  void Jitter::addFrame(AVFrame* frame) {

    if (NULL == frame) {
      RX_ERROR("Cannot add frame, is NULL");
      return;
    }

    /* first frame, remember the time ? */
    if (0 == first_pts) {
      first_pts = (frame->pkt_pts * time_base) * 1000llu * 1000llu * 1000llu;
    }

    int64_t pts = (frame->pkt_pts * time_base) * 1000llu * 1000llu * 1000llu;
    curr_buffer_ns = (pts - first_pts)  - curr_pts;

    if (state == JITTER_STATE_BUFFERING) {
      double s = double(curr_buffer_ns) / 1000000.0;
      RX_VERBOSE("PTS: %llu, first_pts: %llu, curr_pts: %llu, buffer: %f, frames: %lu", pts, first_pts, curr_pts, s, frames.size());
    }

    /* and store! */
    frames.push_back(frame);
  }

  /* @todo - check if the jitter buffer keeps filling up when the timestamps of the incoming frames are incorrect and the number of frames keeps groing and groing ... */

  int Jitter::update() {

    if (NULL == on_frame) {
      RX_ERROR("Doesn't make sense to call Jitter::update() when no on_play_frame set.");
      return -1;
    }
    
    /* not frame yet .. or played back everything */
    if (JITTER_STATE_BUFFERING != state && 0 == frames.size()) {
      RX_VERBOSE("No frames left - we're restarting the playback buffer");
      first_pts = 0;
      curr_pts = 0;
      curr_buffer_ns = 0;
      start_pts = 0;
      state = JITTER_STATE_BUFFERING;
      return 0;
    }

    if (JITTER_STATE_PLAYING != state && curr_buffer_ns < buffer_size_ns) {      
      curr_pts = 0;
      double s = double(curr_buffer_ns) / 1000000.0;
      RX_VERBOSE("We haven't enough frames in our buffer, buffer time: %f ms", s);
      return 0;
    }

    if (0 == start_pts) {
      state = JITTER_STATE_PLAYING;
      start_pts = rx_hrtime();
      if (on_event) {
        on_event(VID_EVENT_START_PLAYBACK, user);
      }
    }

    if (JITTER_STATE_PLAYING != state) {
      return 0;
    }

    curr_pts = rx_hrtime() - start_pts;

    /* find the frame that we need to show. */
    std::deque<AVFrame*>::iterator it = frames.begin();
    int64_t pts;
    while (it != frames.end()) {

      /* get the current frame */
      AVFrame* f = *it;
      pts = ((f->pkt_pts * time_base) * 1000llu * 1000llu * 1000llu) - first_pts;
     
      if (pts > curr_pts) {
        break;
      }

      /* call the callback */
      on_frame(f, user);
      it = frames.erase(it);
        
      /* and free + unref the frame */
      av_frame_free(&f);
    }

    if (0 == frames.size() && on_event && state == JITTER_STATE_PLAYING) {
      /* this will make sure we will buffer more frames. */
      first_pts = pts;
    }

    return 0;
  }

} /* namespace vid */
