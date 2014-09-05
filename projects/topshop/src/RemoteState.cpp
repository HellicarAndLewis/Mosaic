#include <err.h>
#include <stdlib.h>
#include <topshop/RemoteState.h>

namespace top {

  /* ------------------------------------------------------------- */

  static void* remote_thread(void* user);
  static int parse_json(RemoteState* state, std::string json);

  /* ------------------------------------------------------------- */

  RemoteState::RemoteState()
    :is_init(false)
    ,must_stop(true)
    ,must_poll(false)
    ,timeout(0)
    ,delay(10e9) /* poll every X seconds */
  {

    int r;

    r = pthread_mutex_init(&mutex, NULL);
    if (0 != r) {
      RX_ERROR("Cannot init the mutex: %s", strerror(r));
      exit(EXIT_FAILURE);
    }
    
    r = pthread_cond_init(&cond, NULL);
    if (0 != r) {
      RX_ERROR("Cannot init the cond %s", strerror(r));
      exit(EXIT_FAILURE);
    }
  }

  RemoteState::~RemoteState() {
    
    int r;

    if (true == is_init) {
      shutdown();
    }

    is_init = false;
    must_stop = true;
    must_poll = false;

    r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to destroy the mutex: %s", strerror(r));
    }
    
    r = pthread_cond_destroy(&cond);
    if (0 != r) {
      RX_ERROR("Failed to destroy the cond var: %s", strerror(r));
    }
  }

  int RemoteState::init() {

    int r;

    if (true == is_init) {
      RX_ERROR("Already initialized");
      return -1;
    }

    is_init = true;
    must_stop = false;

    r = pthread_create(&thread, NULL, remote_thread, this);
    if (0 != r) {
      RX_ERROR("Cannot start the remote state thread: %s", strerror(r));
      must_stop = true;
      is_init = false;
      return -2;
    }

    return 0;
  }
  
  void RemoteState::update() {
    uint64_t now = rx_hrtime();
    if (now >= timeout) {
      timeout = now + delay;
      lock();
      {
        must_poll = true;
        pthread_cond_signal(&cond);
      }
      unlock();
    }
  }

  int RemoteState::shutdown() {

    if (false == is_init) {
      RX_ERROR("Not initialized, cannot shutdown.");
      return -1;
    }

    lock();
      must_stop = true;
      pthread_cond_signal(&cond);
    unlock();

    RX_VERBOSE("Joining remote state thread.");
    pthread_join(thread, NULL);

    is_init = false;
    return 0;
  }

  /* ------------------------------------------------------------- */

  static void* remote_thread(void* user) {

    RemoteState* rem = static_cast<RemoteState*>(user);
    if (NULL == rem) {
      RX_ERROR("User pointer is invalid in the remote state thread; not supposed to happen.");
      return NULL;
    }

    bool must_stop = false;
    bool must_poll = false;

    while (false == rem->must_stop) {
      rem->lock();
      {
        while (false == rem->must_stop && false == rem->must_poll) {
          pthread_cond_wait(&rem->cond, &rem->mutex);
        }
        must_stop = rem->must_stop;
        must_poll = rem->must_poll;
        rem->must_poll = false;
      }
      rem->unlock();

      if (true == must_stop) {
        RX_VERBOSE("Stopping remote state thread");
        return NULL;
      }

      if (false == must_poll) {
        continue;
      }

      /* check the remote state */
      std::string url = "http://test.localhost/test.json";
      std::string result;
      if (false == rx_fetch_url(url, result)) {
        RX_ERROR("Failed to get the url %s", url.c_str());
        continue;
      }
      
      if (0 == result.size()) {
        RX_ERROR("State is empty.");
        continue;
      }

      parse_json(rem, result);
    }

    return NULL;
  }

  static int parse_json(RemoteState* state, std::string json) {

    if (NULL == state || 0 == json.size()) {
      return -1;
    }

    json_t* root = NULL;
    json_t* el = NULL;
    json_error_t error;

    root = json_loads(json.c_str(), 0, &error);
    if (NULL == root) {
      RX_ERROR("Invalid state json: %s (%d)", error.text, error.line);
      return -2;
    }
    
    el = json_object_get(root, "show_mosaic");
    if (NULL == el) {
      json_decref(root);
      root = NULL;
      return -3;
    }
    
    int show_mosaic = json_integer_value(el);
    RX_VERBOSE("Show mosaic: %d", show_mosaic);

    json_decref(root);
    root = NULL;

    return 0;

  }

  /* ------------------------------------------------------------- */

} /* namespace top */
