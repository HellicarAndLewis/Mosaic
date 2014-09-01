/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/
#ifndef VIDEO_JITTER_H
#define VIDEO_JITTER_H

#include <deque>
#include <vector>
#include <stdint.h>
#include <video/Types.h>
    
extern "C" {
#  include <libavformat/avformat.h>
#  include <libavcodec/avcodec.h>
}

#define JITTER_STATE_NONE 0x00
#define JITTER_STATE_PLAYING 0x01
//#define JITTER_STATE_READY 0x02
#define JITTER_STATE_BUFFERING 0x02

namespace vid {

  typedef void(*jitter_on_frame)(AVFrame* frame, void* user); /* gets called whenever the user needs to play the given frame. */
  
  class Jitter {

  public:
    Jitter();
    ~Jitter();
    int init(double timebase);        /* initialize the jitter buffer using the given timebase in seconds (NOT ns) */
    int shutdown();                   /* resets everything */
    void addFrame(AVFrame* frame);    /* add a frame to the buffer */
    int update();                     /* call this often as it will check if you need to display a new frame. */ 

  public:
    std::deque<AVFrame*> frames;  
    int64_t first_pts;               /* firts pts in frames; used to match the system timeline with the frame timeline */
    int64_t buffer_size_ns;          /* the buffer size we need in nanoseconds */
    int64_t curr_buffer_ns;          /* how many nanoseconds do we have in our frames buffer. */
    int64_t curr_pts;                /* the pts that was last used. */
    int64_t start_pts;               /* when we started playing back */
    double time_base;                /* timebase in seconds */
    int state;

    video_on_event on_event;
    jitter_on_frame on_frame;
    void* user;
  };
} /* namespace vid */

#endif
