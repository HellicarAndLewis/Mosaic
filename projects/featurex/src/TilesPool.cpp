#include <featurex/TilesPool.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  Tile::Tile() {
    reset();
  }

  Tile::~Tile() {
    reset();
  }

  void Tile::reset() {
    if (NULL == pixels) {
      delete[] pixels;
    }
    pixels = NULL;
    width = 0;
    height = 0;
    nchannels = 0;
    capacity = 0;
  }

  /* ---------------------------------------------------------------------------------- */


  TilesPool::TilesPool() 
    :buffer(NULL)
  {
  }

  TilesPool::~TilesPool() {
    shutdown();

    buffer = NULL;
  }

  int TilesPool::init() {

    if (false == fex::config.validateTilePoolSettings()) {
      return -1;
    }

    uint64_t nbytes = fex::config.getTilePoolSizeInBytes();
    if (0 == nbytes) {
      RX_ERROR("Invalid number of bytes for the tile pool size.");
      return -2;
    }

    buffer = (unsigned char*)malloc(nbytes);
    if (NULL == buffer) {
      RX_ERROR("Cannot allocate the buffer for the tiles pool");
      return -3;
    }
    
    RX_VERBOSE("Allocating %llu bytes for the tiles pool.", nbytes);

    return 0;
  }

  int TilesPool::shutdown() {

    if (buffer) {
      RX_VERBOSE("Freeing tiles buffer");
      delete[] buffer;
    }

    buffer = NULL;
    return 0;
  }

  /* ---------------------------------------------------------------------------------- */


} /* namespace fex */