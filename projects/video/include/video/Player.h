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

  Player
  -------

  Uses libav to decode a video stream url in a separate thread and makes sure
  we have a big enough jitter buffer for smooth play back. You can test this with 
  e.g. avconv. To create a "server" to which you can connect with libav/avconv
  use something like this:

  --
    ./avconv -re -i yourvideo.mov -an -v debug -f flv -listen 1 rtmp://localhost/
  --

  Then you can connect to the `rtmp://localhost` stream.

 */
#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <string>
#include <video/Stream.h>
#include <video/Jitter.h>
#include <video/Types.h>
#include <pthread.h>

#define ROXLU_USE_LOG
#include <tinylib.h>

extern "C" {
#  include <libavformat/avformat.h>
#  include <libavcodec/avcodec.h>
}

namespace vid {


  typedef void(*player_on_frame)(AVFrame* frame, void* user);

  class Player {
  public:
    Player();
    ~Player();
    int init(std::string url);        /* initialize the player. */
    int update();                     /* call this often; it will check if the jitter buffer has a new frame that we need to play/show */
    int shutdown();                   /* shutsdown the player - will be called automatically by the thread functions; do not call yourself */
    int stop();                       /* stop playing and shutdown everything nicely */
    int play();                       /* play the stream (init will also start playing directly) */
    void lock();                      /* locks the mutex */
    void unlock();                    /* unlocks the mutex */

  public:
    Stream stream;                    /* decodes an url using libav. */
    Jitter jitter;                    /* the playback buffer to make sure we can playback smoothly */
    std::string url;                  /* the url that we opened. */

    bool is_running;                  /* is set to true when the player thread is running */
    bool must_stop;                   /* is set to true when we must stop the thread. */
    pthread_t thread;                 /* the stream/decoding thread */
    pthread_mutex_t mutex;            /* secures the jitter buffer */

    /* callback */
    player_on_frame on_frame;         /* gets called when a new video frame needs to be shown. */
    video_on_event on_event;          /* gets called when the stream e.g. starts, stops, or eof is found. */
    void* user;                       /* gets passed into on_frame */
  };

  inline void Player::lock() {

    if (false == is_running) {
      RX_ERROR("Cannot lock because we're not running and the mutex is invalid!");
      return;
    }

    pthread_mutex_lock(&mutex);
  }

  inline void Player::unlock() {

    if (false == is_running) {
      RX_ERROR("Cannot unlock because we're not running and the mutex is invalid!");
      return;
    }

    pthread_mutex_unlock(&mutex);
  }
} /* namespace vid */

#endif
