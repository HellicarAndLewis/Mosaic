#include <mosaic/ImageLoader.h>

namespace mos {

  /* --------------------------------------------------------------------- */

  static void* image_thread(void* user);

  /* --------------------------------------------------------------------- */

  ImageTask::ImageTask() 
    :pixels(NULL)
    ,width(0)
    ,height(0)
    ,channels(0)
    ,capacity(0)
    ,nbytes(0)
    ,type(IMAGE_TASK_NONE)
    ,is_free(true)
  {
    reset();
  }

  ImageTask::~ImageTask() {

    RX_VERBOSE("Freeing image task, with %d allocated bytes.", capacity);

    reset();

    if (pixels) {
      delete[] pixels;
    }

    pixels = 0;
    capacity = 0;
  }

  int ImageTask::reset() {
    /* make free again, but don't free memory */
    type = IMAGE_TASK_NONE;
    width = 0;
    height = 0;
    channels = 0;
    is_free = true;
    nbytes = 0;

    return 0;
  }

  /* ---------------------------------------------------------------------------------- */

  ImageLoader::ImageLoader() 
    :is_running(false)
    ,must_stop(true)
    ,on_loaded(NULL)
    ,user(NULL)
  {
    /* the mutex and cond var are kept for the whole lifetime of the analyzer */
    if (0 !=  pthread_mutex_init(&mutex, NULL)) {
      RX_ERROR("Cannot initialize the mutex.");
    }

    if (0 != pthread_cond_init(&cond, NULL)) {
      RX_ERROR("Cannot initilialize the cond var.");
    }    
  }

  ImageLoader::~ImageLoader() {

    if (is_running) {
      shutdown();
    }

    is_running = false;
    must_stop = true;

    /* and destory the mutex + cond var */
    int r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot destroy mutex: %d", r);
    }

    r = pthread_cond_destroy(&cond);
    if (0 != r) {
      RX_ERROR("Cannot destory condiation variable: %d", r);
    }

    /* mega safe ^.^ */
    if (0 != tasks.size()) {
      RX_ERROR("We still have tasks! not supposed to happen");
      for (size_t i = 0; i < tasks.size(); ++i) {
        delete tasks[i];
      }
      tasks.clear();
    }
  }

  int ImageLoader::init() {

    if (true == is_running) {
      RX_ERROR("Already running!");
      return -1;
    }

    /* start the laoder thread */
    must_stop = false;

    if (0 != pthread_create(&thread, NULL, image_thread, this)) {
      RX_ERROR("Cannot initialize the thread for the loader.");
      return -6;
    }

    return 0;
  }

  int ImageLoader::load(std::string filepath) {

    if (0 == filepath.size()) {
      RX_ERROR("Invalid file. size is 0.");
      return -1;
    }

    if (false == rx_file_exists(filepath)) {
      return -2;
    }

    std::string ext = rx_get_file_ext(filepath);
    if (ext != "jpg" && ext != "png") {
      RX_VERBOSE("Received an directory event for a non-image file: %s", filepath.c_str());
      return -3;
    }

    /* add this file to the load queue. */
    ImageTask* task = getFreeTask();
    if (NULL == task) {
      RX_ERROR("Cannot get a free task. Out of mem or error. Not using this image: %s", filepath.c_str());
      return -5;
    }

    task->filepath = filepath;
    task->extension = ext;
    task->type = IMAGE_TASK_LOAD;
    
    return addWork(task);
  }

  int ImageLoader::shutdown() {

    int r = 0;

    if (false == is_running) {
      RX_ERROR("Cannot shutdown the image loader because the thread is not running. Did you call init()");
      return -1;
    }

    /* signal the stop */
    must_stop = true;
    lock();
      pthread_cond_signal(&cond);
    unlock();

    /* wait till the loader thread stopped. */
    pthread_join(thread, NULL);

    /* cleanup */
    for (size_t i = 0; i < tasks.size(); ++i) {
      delete tasks[i];
    }
    
    tasks.clear();
    work.clear();

    return 0;
  }

  ImageTask* ImageLoader::getFreeTask() {
    ImageTask* task = NULL;
    size_t num_tasks = 0;

    /* find a free task */
    lock();
    {
      num_tasks = tasks.size();

      for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i]->is_free) {
          task = tasks[i];
          task->reset();
          task->is_free = false;
          task->type = IMAGE_TASK_LOAD;
        }
      }
    }
    unlock();
    
    if (NULL == task) {
      /* create a new task */
      task = new ImageTask();
      if (NULL == task) {
        RX_ERROR("Cannot allocate a new task; out of memory?");
        return NULL;
      }

      /* just give some feedback. */
      if (100 < num_tasks) {
        RX_ERROR("Already created %lu tasks, maybe you need to process more or adjust the number of tasks.", num_tasks);
      }

      task->is_free = false;
      task->type = IMAGE_TASK_LOAD;
      
      /* add the new tasks. */
      lock();
        tasks.push_back(task);
      unlock();
    }

    return task;
  }

  int ImageLoader::addWork(ImageTask* task) {

    if (NULL == task) {
      RX_ERROR("Invalid task given to the loader: NULL");
      return -1;
    }
    
    lock();
    {
      work.push_back(task);
      if (0 != pthread_cond_signal(&cond)) {
        RX_ERROR("Signaling gave an unexpected result");
      }
    }
    unlock();

    return 0;
  }

  /* --------------------------------------------------------------------- */

  static void* image_thread(void* user) {

    /* get a handle to the loader */
    ImageLoader* loader = static_cast<ImageLoader*>(user);
    if (NULL == loader) {
      RX_ERROR("The image loader thread didn't get a loader object - not supposed to happen!");
      exit(EXIT_FAILURE);
    }

    loader->is_running = true;

    std::vector<ImageTask*> todo;

    while (false == loader->must_stop) {

      /* wait for work */
      loader->lock();
      {
        while (0 == loader->work.size() && loader->must_stop == false) {
          pthread_cond_wait(&loader->cond, &loader->mutex);
        }
        std::copy(loader->work.begin(), loader->work.end(), std::back_inserter(todo));
        loader->work.clear();
      }
      loader->unlock();

      /* stop */
      if (loader->must_stop) {
        break;
      }

      /* spurious wake-up */
      if (0 == todo.size()) {
        continue;
      }

      /* work! */
      for (size_t i = 0; i < todo.size(); ++i) {

        ImageTask* t = todo[i];

        if (IMAGE_TASK_LOAD != t->type) {
          RX_VERBOSE("Unhandled image loader task: %d", t->type);
          continue;
        }

        if (0 == t->filepath.size()) {
          RX_ERROR("The filepath is invalid, not supposed to happen!");
          continue;
        }

        if (false == rx_file_exists(t->filepath)) {
          RX_ERROR("Cannot find the file to load: %s", t->filepath.c_str());
          continue;
        }

        /* load the image */
        if (t->extension == "jpg") {
          t->nbytes = rx_load_jpg(t->filepath, &t->pixels, t->width, t->height, t->channels, &t->capacity);
        }
        else if (t->extension == "png") {
          t->nbytes = rx_load_png(t->filepath, &t->pixels, t->width, t->height, t->channels, &t->capacity);
        }
        else {
          RX_ERROR("Unhandled image extension - not supposed to happen");
          continue;
        }
        
        if (0 >= t->nbytes) {
          RX_ERROR("Something went wrong while loading the file %s", t->filepath.c_str());
          continue;
        }

        RX_VERBOSE("Loaded: %s", t->filepath.c_str());
        RX_VERBOSE("width: %d, height: %d, channels: %d, capacity: %d, nbytes: %d", 
                   t->width, t->height, t->channels, t->capacity, t->nbytes);

        /* notify the user that we've loaded an image */
        if (NULL != loader->on_loaded) {
          loader->lock();
          loader->on_loaded(t, loader->user);
          loader->unlock();
        }
      }

      /* make free again. */
      loader->lock();
      for (size_t i = 0; i < todo.size(); ++i) {
        todo[i]->reset();
      }
      loader->unlock();
      
      /* and cleanup */
      todo.clear();
    }

    loader->is_running = false;

    return NULL;
  }

  /* --------------------------------------------------------------------- */

} /* namespace mos */
