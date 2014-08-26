#ifndef ROXLU_TILES_POOL_H
#define ROXLU_TILES_POOL_H

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
    int width;
    int height;
    int nbytes;
    int nchannels;
    int capacity;
    unsigned char* pixels;
  };

  /* ---------------------------------------------------------------------------------- */

  class TilesPool {
  public:
    TilesPool();             /* create a pool that can handle ntiles of images. */
    ~TilesPool();            /* cleanup all allocated memory */
    int init();              /* initialize; allocate a huge memory buffer to store all image data. */ 
    int shutdown();          /* free, cleanup. destroys all allocated objects. */

  public:
    std::vector<Tile*> tiles;
    unsigned char* buffer;
  };

} /* namespace fex */

#endif
