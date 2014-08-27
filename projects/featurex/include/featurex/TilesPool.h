#ifndef ROXLU_TILES_POOL_H
#define ROXLU_TILES_POOL_H

#include <pthread.h>
#include <vector>
#include <featurex/Config.h>
#include <featurex/Descriptor.h>

#define ROXLU_USE_LOG
#define ROXLU_USE_PNG
#define ROXLU_USE_JPG
#include <tinylib.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  class Tile {
  public:
    Tile();
    ~Tile();
    void reset();
    
  public:
    uint32_t descriptor_id;
    int is_free;
    int width;
    int height;
    int nbytes;
    int nchannels;
    int capacity;
    unsigned char* pixels;
  };

  /* ---------------------------------------------------------------------------------- */

  class TileTask {
  public:
    TileTask();
    ~TileTask();

  public:
    bool is_free;
    std::string filename;
    uint32_t descriptor_id;
  };

  /* ---------------------------------------------------------------------------------- */

  class TilesPool {
  public:
    TilesPool();                                /* create a pool that can handle ntiles of images. */
    ~TilesPool();                               /* cleanup all allocated memory */
    int init();                                 /* initialize; allocate a huge memory buffer to store all image data. */ 
    int shutdown();                             /* free, cleanup. destroys all allocated objects. */
    int loadDescriptorTile(Descriptor& desc);   /* loads the tile for the given descriptor. */
    void lock();
    void unlock();

    Tile* getTileForDescriptorID(uint32_t id);
    Tile* getFreeTile();                        /* get a free tile that we can "fill" with pixel data */

  private:
    void freeTiles();
    void freeTasks();
    TileTask* getFreeTask();                    /* get a free task; if none was found we allocate a new one. */

  public:
    std::vector<Tile*> tiles;
    std::vector<TileTask*> tasks;               /* contains all of the allocated tasks */
    std::vector<TileTask*> work;                /* contains tasks which is_free member is set to false and which need to be handled in the thread. */ 
    unsigned char* buffer;

    /* threading */
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool must_stop;
    bool is_running;
  };

  inline void TilesPool::lock() {
    if (0 != pthread_mutex_lock(&mutex)) {
      RX_WARNING("Failed to lock the mutex");
    }
  }

  inline void TilesPool::unlock() {
    if (0 != pthread_mutex_unlock(&mutex)) {
      RX_WARNING("Failed to unlock the mutex");
    }
  }

} /* namespace fex */

#endif
