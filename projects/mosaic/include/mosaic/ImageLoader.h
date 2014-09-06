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

  ImageLoader
  -----------
  
  Basic threaded image loader. Each new 'load' adds a task to the loader thread. We
  try to reuse allocated bytes so we're fast. But you need to make sure that we're 
  not allocating to many memory.

  Usage:
  ------

  ````c++
          ImageLoader loader;
        
          loader.init();                   // start thread
          loader.on_loaded = my_callback;  // set callback 
          loader.user = mydata;            // set data that is passed into the callback. 
          ...
          loader.load(filepath);           // load a file in the thread
          ...
          loader.shutdown();               // gracefully shutdown, stop the thread and free mem.
  ````

*/
#ifndef MOSAIC_IMAGE_LOADER_H
#define MOSAIC_IMAGE_LOADER_H

extern "C" {
#  include <uv.h>
}

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#include <tinylib.h>
#include <err.h>

#define IMAGE_TASK_NONE 0x00
#define IMAGE_TASK_LOAD 0x01      /* load a new image */

namespace mos {

  /* ---------------------------------------------------------------------------------- */

  class ImageTask;
  typedef void(*imageloader_on_loaded)(ImageTask* task, void* user); /* gets called from the loader thread when we've loaded a new image - you need to copy the pixels but be aware that we're running in a separate thread. */

  /* ---------------------------------------------------------------------------------- */

  class ImageTask {
  public:
    ImageTask();
    ~ImageTask();
    int reset();                   /* resets all members so this task can be reused. it will not clear/reset all members because some need to exist as long as this object does. */

    /* thread related */
    bool is_free;                  /* is the task free */
    int type;                      /* the task type; probably just IMAGE_TASK_LOAD */
    
    /* image related */
    std::string filepath;          /* path to the directory */
    std::string extension;         /* file extension */ 
    unsigned char* pixels;         /* the pixel data */
    int width;                     /* width of the image */
    int height;                    /* height of the image. */
    int channels;                  /* number of channels in the image. */ 
    int capacity;                  /* how many bytes can be stored in pixels */
    int nbytes;                    /* how many bytes were loaded. */
  };

  /* ---------------------------------------------------------------------------------- */

  class ImageLoader {

  public:
    ImageLoader();
    ~ImageLoader();
    int init();                            /* initialize */
    int load(std::string filepath);        /* load the given filepath in a separate thread */
    int shutdown();                        /* cleanup, stop thread, frees allocated mem. */
    void lock();                           /* locks our mutex for shared resources. */
    void unlock();                         /* unlock the mutex */

    ImageTask* getFreeTask();              /* used internally; creates or returns an free task */
    int addWork(ImageTask* task);          /* add some work to the queue; used internally. */

  public:

    /* threading */
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool is_running;
    bool must_stop;
    std::vector<ImageTask*> tasks;
    std::vector<ImageTask*> work;

    /* callback */
    imageloader_on_loaded on_loaded;      /* gets called when an image has been loaded. */
    void* user;                           /* gets passed into the on_loaded function. */
  };

  /* ---------------------------------------------------------------------------------- */

  inline void ImageLoader::lock() {
    int r = pthread_mutex_lock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to lock the mutex in the image loader: %s", strerror(r));
    }
  }

  inline void ImageLoader::unlock() {
    int r = pthread_mutex_unlock(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to unlock the mutex in the image loader: %s", strerror(r));
    }
  }

} /* namespace mos */

#endif
