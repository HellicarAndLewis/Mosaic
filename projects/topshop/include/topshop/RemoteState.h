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

   RemoteState
   -----------

   This is used to fire a request to the remote admin to get the current 
   application settings. The admin can control certain features of the 
   installation. We've chosen to poll every X-seconds so we don't have to 
   think about keeping track of open/closed sockets. 

*/

#ifndef ROXLU_REMOTE_STATE_H
#define ROXLU_REMOTE_STATE_H

#include <pthread.h>

#define ROXLU_USE_CURL
#define ROXLU_USE_LOG
#include <tinylib.h>
#include <jansson.h>

namespace top {

  class RemoteState {
  public:
    RemoteState();
    ~RemoteState();
    int init();
    void update();
    int shutdown();
    void lock();
    void unlock();

  public:
    bool is_init;
    bool must_stop;
    bool must_poll;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint64_t timeout;
    uint64_t delay;
  };

  inline void RemoteState::lock() {
    int r = pthread_mutex_lock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to lock the mutex: %s", strerror(r));
    }
  }

  inline void RemoteState::unlock() {
    int r = pthread_mutex_unlock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to lock the mutex: %s", strerror(r));
    }
  }



  } /* namespace top */

#endif
