#include <featurex/TilesPool.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  static void* tilespool_thread(void* user);

  /* ---------------------------------------------------------------------------------- */

  Tile::Tile() {
    reset();
  }

  Tile::~Tile() {
    reset();
  }

  void Tile::reset() {
    pixels = NULL;      /* points into TilesPool.buffer which frees it */
    width = 0;
    height = 0;
    nchannels = 0;
    capacity = 0;
    is_free = true;
    descriptor_id = 0;
  }

  /* ---------------------------------------------------------------------------------- */

  TileTask::TileTask() 
    :is_free(true)
    ,descriptor_id(0)
  {
  }

  TileTask::~TileTask() {
    is_free = false;
    descriptor_id = 0;
    filename.clear();
  }

  /* ---------------------------------------------------------------------------------- */


  TilesPool::TilesPool() 
    :buffer(NULL)
  {

    /* we keep the mutex + cond for the duration of the TilesPool */
    if (0 != pthread_mutex_init(&mutex, NULL)) {
      RX_ERROR("Cannot initialize the mutex");
    }

    if (0 != pthread_cond_init(&cond, NULL)) {
      RX_ERROR("Cannot initalize the cond var");
    }

  }

  TilesPool::~TilesPool() {
    shutdown();

    if (0 != pthread_mutex_destroy(&mutex)) {
      RX_ERROR("Cannot destroy the mutex");
    }

    if (0 != pthread_cond_destroy(&cond)) {
      RX_ERROR("Cannot destroy the cond var.");
    }

    buffer = NULL;
  }

  int TilesPool::init() {
    int r = 0;
    uint64_t nbytes;
    uint32_t bytes_per_tile;
    uint32_t buffer_offset = 0;
    int i;

    if (false == fex::config.validateTilePoolSettings()) {
      return -1;
    }

    /* create the huge buffer that will hold the tile pixels. */
    nbytes = fex::config.getTilePoolSizeInBytes();
    if (0 == nbytes) {
      RX_ERROR("Invalid number of bytes for the tile pool size.");
      return -2;
    }

    buffer = (unsigned char*)malloc(nbytes);
    if (NULL == buffer) {
      RX_ERROR("Cannot allocate the buffer for the tiles pool");
      return -3;
    }

    /* create the tiles */
    bytes_per_tile = fex::config.file_tile_width * fex::config.file_tile_height * 4; 
    for (i = 0; i < fex::config.memory_pool_size; ++i) {

      Tile* tile = new Tile();
      if (NULL == tile) {
        RX_WARNING("Cannot allocate a new tile. Out of memory?");
        continue;
      }

      tile->pixels = buffer + buffer_offset;
      tile->capacity = bytes_per_tile;
      tiles.push_back(tile);

      buffer_offset += bytes_per_tile;
    }

    RX_VERBOSE("Allocating %llu bytes for the tiles pool.", nbytes);

    must_stop = false;
    
    if (0 != pthread_create(&thread, NULL, tilespool_thread, this)) {
      RX_ERROR("Cannot initialize the tile pool thread");
      r = -6;
      goto error;
    }

    is_running = true;

    return r;

  error:
    if (NULL != buffer) {
      free(buffer);
      buffer = NULL;
    }

    must_stop = true;
    is_running = false;

    freeTiles();    

    return r;
  }

  int TilesPool::shutdown() {

    /* stop the thread. */
    if (is_running) {
      RX_VERBOSE("Joining thread");

      must_stop = true;
      lock();
        pthread_cond_signal(&cond);
      unlock();

      pthread_join(thread, NULL);
    }
   
    freeTiles();
    freeTasks();

    /* and cleanup our huge buffer. */
    if (buffer) {
      RX_VERBOSE("Freeing tiles buffer");
      free(buffer);
    }

    buffer = NULL;
    is_running = false;
    must_stop = true;

    return 0;
  }

  void TilesPool::freeTasks() {
    RX_VERBOSE("Destroying %lu tasks", tasks.size());
    for (size_t i = 0; i < tasks.size(); ++i) {
      delete tasks[i];
    }
    tasks.clear();
  }

  void TilesPool::freeTiles() {
    RX_VERBOSE("Destroying %lu tiles", tiles.size());
    for (size_t i = 0; i < tiles.size(); ++i) {
      delete tiles[i];
    }
    tiles.clear();
  }

  int TilesPool::loadDescriptorTile(Descriptor& desc) {

    int r = 0;

    if (0 == desc.getFilename().size()) {
      RX_ERROR("Trying to laod a descriptor tile but the filename is not set!");
      return -1;
    }

    if (0 == desc.id) {
      RX_ERROR("The ID of the descriptor is 0, seems like an invalid descriptor. not supposed to happen.");
      return -2;
    }

    TileTask* task = getFreeTask();
    if (NULL == task) {
      RX_ERROR("Cannot create a new task and so not load new pixels");
      return -3;
    }

    task->filename = desc.getFilename();
    task->descriptor_id = desc.id;

    lock();
    {
      work.push_back(task);
      
      r = pthread_cond_signal(&cond);
      if (0 != r) {
        RX_ERROR("Error while trying to signal the condition var: %d", r);
      }
    }
    unlock();

    return r;
  }


  /* finds a free tasks, or creates a new one. */
  TileTask* TilesPool::getFreeTask() {
    TileTask* task = NULL;

    /* find a free one */
    lock();
    {
      for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i]->is_free) {
          task = tasks[i];
          task->is_free = false;
          break;
        }
      }
    }
    unlock();

    /* create a new one. */
    if (NULL == task) {
      RX_VERBOSE("No free task found, creating a new one.");

      task = new TileTask();
      if (NULL == task) {
        RX_ERROR("Cannot allocate a new task; out of memory? Try reducing the specs.");
        return NULL;
      }

      task->is_free = false;

      lock();
        tasks.push_back(task);
      unlock();

      /* just give a working when we're creating many tasks */
      if (100 <= tasks.size()) {
        RX_WARNING("We have %lu tasks in the task list; this is okay but make sure that we can process all tasks; maybe reduce some specs in the config.", tasks.size());
      }
    }

    return task;
  }

  Tile* TilesPool::getFreeTile() {
    Tile* tile = NULL;
    lock();
    {
      for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i]->is_free) {
          tile = tiles[i];
          tile->is_free = false;
          break;
        }
      }
    }
    unlock();

    return tile;
  }

  Tile* TilesPool::getTileForDescriptorID(uint32_t id) {
    if (0 == id) {
      RX_ERROR("Cannot get tile because the ID is invalid (0).");
      return NULL;
    }

    Tile* tile = NULL;
    lock();
    {
      for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i]->descriptor_id == id) {
          tile = tiles[i];
          break;
        }
      }
    }
    unlock();

    return tile;
  }

  /* ---------------------------------------------------------------------------------- */

  static void* tilespool_thread(void* user) {

    /* get our TilesPool ptr */
    TilesPool* pool = static_cast<TilesPool*>(user);
    if (NULL == pool) {
      RX_ERROR("The TilePool user ptr is invalid in the tiles pool thread. Not supposed to happen.");
      ::exit(EXIT_FAILURE);
    }

    std::vector<TileTask*> todo;

    while (false == pool->must_stop) {

      /* wait for work */
      pool->lock();
      {
        while (0 == pool->work.size() && false == pool->must_stop) {
          pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        std::copy(pool->work.begin(), pool->work.end(), std::back_inserter(todo));
        pool->work.clear();
      }
      pool->unlock();

      /* stop? */
      if (pool->must_stop) {
        break;
      }

      if (0 == todo.size()) {
        continue;
      }

      /* do the work! */
      for (size_t i = 0; i < todo.size(); ++i) {

        TileTask* task = todo[i];
        if (0 == task->filename.size()) {
          RX_ERROR("The filename size of the tiletask is 0; not supposed to happen.");
          continue;
        }

        /* do we support the file type? */
        std::string ext = rx_get_file_ext(task->filename);
        if (ext != "jpg" && ext != "png") {
          RX_ERROR("Unsupported file type");
          continue;
        }

        /* does it still exist on disk? */
        std::string tile_filepath = fex::config.resized_filepath +task->filename;
        if (false == rx_file_exists(tile_filepath)) {
          RX_ERROR("Cannot find: %s", tile_filepath.c_str());
          continue;
        }
        
        Tile* tile = pool->getFreeTile();
        if (NULL == tile) {
          RX_ERROR("Cannot get a free tile - not loading the tile pixels.");
          continue;
        }

        /* load the image */
        //RX_VERBOSE("Filename: %s", tile_filepath.c_str());
#if 1
        int curr_capacity = tile->capacity;
        int bytes_loaded = 0;
        if (ext == "jpg") {
          bytes_loaded = rx_load_jpg(tile_filepath, &tile->pixels, tile->width, tile->height, tile->nchannels, &tile->capacity);
        }
        else if (ext == "png") {
          bytes_loaded = rx_load_png(tile_filepath, &tile->pixels, tile->width, tile->height, tile->nchannels, &tile->capacity);
        }

        if (bytes_loaded > curr_capacity) {
          RX_ERROR("We read more bytes then the actual capacity of a tile. This means that the image size is different then the size we expect it to be. This results in faulty buffers and undefined behavior");
        }

        tile->nbytes = bytes_loaded;
        tile->descriptor_id = task->descriptor_id;

        // RX_VERBOSE("Loaded image: %s, bytes loaded: %d, width: %d, height: %d, channels: %d, capacity: %d", task->filename.c_str(), tile->nbytes, tile->width, tile->height, tile->nchannels, tile->capacity);
#endif
      }
      todo.clear();

      /* and free all tasks again. */
      pool->lock();
      {
        for (size_t i = 0; i < todo.size(); ++i) {
          todo[i]->is_free = true;
        }
      }
      pool->unlock();
    }

    return NULL;
  }

  /* ---------------------------------------------------------------------------------- */

} /* namespace fex */
