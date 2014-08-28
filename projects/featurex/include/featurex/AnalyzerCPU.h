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

#ifndef ROXLU_ANALYZER_CPU_H
#define ROXLU_ANALYZER_CPU_H

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#define ROXLU_USE_MATH
#include <tinylib.h>

#include <string>
#include <vector>
#include <pthread.h>
#include <featurex/Descriptor.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  enum {
    ANA_TASK_NONE,
    ANA_TASK_ANALYZE
  };

  /* ---------------------------------------------------------------------------------- */

  class AnalyzerTask {
  public:
    AnalyzerTask();
    ~AnalyzerTask();
    void reset();               /* resets the members so this task can be reused. We will not free allocated mem. */

  public:
    int type;                   /* what kind of task */
    bool is_free;               /* when is_free, it can be used as a task for the analyzer thread */
    int capacity;               /* the capacity of pixels */

    /* image info */
    std::string filepath;       /* path to the image we want to analyze */
    int width;                  /* width of the image */     
    int height;                 /* height of the image */
    int channels;               /* number of channels in the image */
    unsigned char* pixels;      /* the buffer that is allocated */
    int nbytes;                 /* the number of bytes in pixels, (capacity can be bigger) */ 
  };

  /* ---------------------------------------------------------------------------------- */

  typedef void(*on_analyzed_callback)(Descriptor& desc, void* user); /* gets called from the analyzed thread whenever we have analyzed a new image. */

  class AnalyzerCPU {
    
  public:
    AnalyzerCPU();
    ~AnalyzerCPU();
    int init();                                  /* initialize, start the thread */
    int join();                                  /* join the analyzer thread */
    int shutdown();                              /* shutdown and stop the thread */
    int analyze(std::string filepath);           /* analyze the given file. */
    void lock();                                 /* lock the mutex; syncs the tasks */
    void unlock();                               /* unlock the mutex. */
    int saveDescriptors();                       /* will store the descriptors into a file in the data dir. */ 
    int loadDescriptors();                       /* load the previously stored descriptors. */
    int executeAnalyzeTask(AnalyzerTask* task);  /* shouldn't be called by a user - is called by the thread */
    
  private:
    AnalyzerTask* getFreeTask();

  public:
    /* threading, consumer/producer */
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool is_running;
    bool must_stop;
    std::vector<AnalyzerTask*> tasks;
    std::vector<AnalyzerTask*> work;
    std::vector<Descriptor> descriptors;

    /* callback */
    on_analyzed_callback on_analyzed;
    void* user;

    /* convert from N channel pixel buffer to 4 */
    unsigned char* rgba;
  };

  /* ---------------------------------------------------------------------------------- */

  inline void AnalyzerCPU::lock() {
    pthread_mutex_lock(&mutex);
  }

  inline void AnalyzerCPU::unlock() {
    pthread_mutex_unlock(&mutex);
  }


} /* namespace fex */
#endif
