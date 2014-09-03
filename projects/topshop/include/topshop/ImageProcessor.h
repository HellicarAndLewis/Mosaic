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


  ImageProcessor
  --------------

  The ImageProcessor executes the preprocess shell scripts for the left
  and right grids in a separate thread so creating the images will not stall 
  the application. 

*/

#ifndef ROXLU_TOPSHOP_IMAGE_PROCESSOR_H
#define ROXLU_TOPSHOP_IMAGE_PROCESSOR_H

#define ROXLU_USE_LOG
#include <tinylib.h>

#include <pthread.h>
#include <topshop/ImageCollector.h>

namespace top {

  struct ProcessTask {
    CollectedFile file;
  };

  class ImageProcessor {
  public:
    ImageProcessor();
    ~ImageProcessor();
    int init();
    int shutdown();
    void lock();
    void unlock();
    int process(CollectedFile file);

  public:
    bool is_running;
    bool must_stop;
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    std::vector<ProcessTask*> tasks;
  };

  inline void ImageProcessor::lock() {
    int r = pthread_mutex_lock(&mutex);
    if (0 != r) {
      RX_ERROR("Lock failed: %s", strerror(r));
    }
  }

  inline void ImageProcessor::unlock() {
    int r = pthread_mutex_unlock(&mutex);
    if (0 != r) {
      RX_ERROR("Unlock failed: %s", strerror(r));
    }
  }

} /* namespace top */

#endif


