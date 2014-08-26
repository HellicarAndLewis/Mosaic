#include <unistd.h>
#include <stdlib.h>
#include <featurex/Config.h>
#include <featurex/AnalyzerCPU.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  static void* analyzer_cpu_thread(void* user);

  /* ---------------------------------------------------------------------------------- */

  AnalyzerTask::AnalyzerTask()
    :type(ANA_TASK_NONE)
    ,is_free(true)
    ,capacity(0)
    ,width(0)
    ,height(0)
    ,channels(0)
    ,pixels(NULL)
    ,nbytes(0)
  {
  }

  AnalyzerTask::~AnalyzerTask() {

    RX_VERBOSE("Freeing analyzer task, with %d allocated bytes.", capacity);

    reset();

    if (pixels) {
      delete[] pixels;
    }

    pixels = NULL;
    capacity = 0;
  }
  
  void AnalyzerTask::reset() {
    type = ANA_TASK_NONE;
    width = 0;
    height = 0;
    channels = 0;
    is_free = true;
    nbytes = 0;
  }

  /* ---------------------------------------------------------------------------------- */

  AnalyzerCPU::AnalyzerCPU()
    :must_stop(true)
    ,is_running(false)
    ,on_analyzed(NULL)
    ,user(NULL)
  {
  }

  AnalyzerCPU::~AnalyzerCPU() {
    
    /* stop thread if it's still running */
    if (is_running) {
      shutdown();
    }

    on_analyzed = NULL;
    user = NULL;
    must_stop = true;
    is_running = false;
  }

  int AnalyzerCPU::init() {

    if (true == is_running) {
      RX_ERROR("Analyzer is already running.");
      return -1;
    }

    if (0 == fex::config.resized_filepath.size()) {
      RX_ERROR("The fex::config.resized_filepath is not set!");
      return -2;
    }

    /* @todo - destroy mutex, cond, thread. */
    if (0 !=  pthread_mutex_init(&mutex, NULL)) {
      RX_ERROR("Cannot initialize the mutex.");
      return -3;
    }

    if (0 != pthread_cond_init(&cond, NULL)) {
      RX_ERROR("Cannot initilialize the cond var.");
      return -4;
    }

    must_stop = false;
    if (0 != pthread_create(&thread, NULL, analyzer_cpu_thread, this)) {
      RX_ERROR("Cannot initialize the thread for the sender.");
      return -5;
    }
  
    is_running = true;
    return 0;
  }

  int AnalyzerCPU::shutdown() {

    int r;

    if (false == is_running) {
      RX_ERROR("Cannot shutdown the analyzer, it's not running.");
      return -1;
    }

    must_stop = true;

    lock();
    {
      pthread_cond_signal(&cond);
    }
    unlock();

    RX_VERBOSE("Shutting down thread.");
    pthread_join(thread, NULL);

    r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot destroy mutex: %d", r);
    }

    r = pthread_cond_destroy(&cond);
    if (0 != r) {
      RX_ERROR("Cannot destory condiation variable: %d", r);
    }

    for (size_t i = 0; i < tasks.size(); ++i) {
      delete tasks[i];
    }

    tasks.clear();
    work.clear();

    return 0;
  }

  int AnalyzerCPU::join() {

    if (false == is_running) {
      RX_ERROR("Cannot join the thread; it's not running");
      return -1;
    }

    pthread_join(thread, NULL);
    return 0;
  }

  int AnalyzerCPU::analyze(std::string filepath) {

    /* validate */
    if (0 == filepath.size()) {
      RX_ERROR("Invalid filepath given.");
      return -1;
    }
    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Cannot find file: %s", filepath.c_str());
      return -2;
    }

    std::string ext = rx_get_file_ext(filepath);
    if (ext != "jpg" && ext != "png") {
      RX_ERROR("Unsupported image format: %s", filepath.c_str());
      return -3;
    }

    /* get a free task */
    AnalyzerTask* task = getFreeTask();
    if (NULL == task) {
      RX_WARNING("There are no free tasks and out-of-memory; we have to skip this one now: %s", filepath.c_str());
    }

    /* set the task info. */
    task->filepath = filepath;
    task->type = ANA_TASK_ANALYZE;

    /* notify our thread that we have some work. */
    lock();
    {
      work.push_back(task);
      pthread_cond_signal(&cond);
    }
    unlock();
  
    return 0;
  }

  /* Get a unused task or creates a new one */
  AnalyzerTask* AnalyzerCPU::getFreeTask() {
    AnalyzerTask* result = NULL;
    size_t num_tasks = 0;

    lock();
    {
      num_tasks = tasks.size();

      for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i]->is_free) {
          result = tasks[i];
          break;
        }
      }
    }
    unlock();

    /* create a new task; when we can't we have to skip this one. */
    if (NULL == result) {
      RX_VERBOSE("Creating new task; all are in use.");

      result = new AnalyzerTask();
      if (NULL == result) {
        RX_ERROR("Out of memory while trying to create a new task. Have to wait for a task to become available.");
        return NULL;
      }

      /* and append to the list */
      lock();
        tasks.push_back(result);
      unlock();
    }

    /* log a message; just a bit of extra info - this probably will never happen. */
    if (100 < num_tasks) {
      RX_WARNING("There are currently %lu tasks created; are we freeing them? Or do we get too many images?", num_tasks);
    }

    result->is_free = false;

    return result;
  }

  /*
    IMPORTANT: This is called from the thread and this is where we will
    analyze the images for the given task.
   */
  int AnalyzerCPU::executeAnalyzeTask(AnalyzerTask* task) {

    std::string ext;
    int ret; 
    int i, j, dx;
    uint64_t r = 0;
    uint64_t g = 0;
    uint64_t b = 0;
    
    /* validate */
    if (NULL == task) {
      RX_ERROR("Invalid task");
      return -1;
    }
    if (0 == task->filepath.size()) {
      RX_ERROR("Invalid filepath in task");
      return -2;
    }

    /* first resize! */
    std::string filename = rx_strip_dir(task->filepath);
    std::string resized_filepath = fex::config.resized_filepath +filename;
    std::string command = "./preprocess.sh " +task->filepath ;
    if (0 == filename.size()) {
      RX_ERROR("Filename is invalid.");
      return -3;
    }
    system(command.c_str());

    /* check if the resized file was created. */
    if (false == rx_file_exists(resized_filepath)) {
      RX_ERROR("Cannot find the resized file: %s", resized_filepath.c_str());
      return -4;
    }

    RX_VERBOSE("Processing %s", task->filepath.c_str());

    /* load the image resized image */
    ext = rx_get_file_ext(resized_filepath);
    if (ext == "jpg") {
      ret = rx_load_jpg(resized_filepath, &task->pixels, task->width, task->height, task->channels, &task->capacity);
    }
    else if(ext == "png") {
      ret = rx_load_png(resized_filepath, &task->pixels, task->width, task->height, task->channels, &task->capacity);
    }
    else {
      RX_ERROR("Unsupported file extension: %s\n", ext.c_str());
    }
    if (ret < 0) {
      RX_ERROR("Something went wrong with loading %s", resized_filepath.c_str());
      return -3;
    }


    RX_VERBOSE("Loaded %s, %d x %d, channels: %d, bytes allocated %d", 
               resized_filepath.c_str(),
               task->width,
               task->height,
               task->channels,
               task->capacity);

    if (3 != task->channels) {
      RX_WARNING("We're not analyzing the image because the number of channels is not yet supported: %d", task->channels);
      return -4;
    }

    /* AVERAGE COLORS */
    for (i = 0; i < task->width; ++i) {
      for (j = 0; j < task->height; ++j) {
        dx = j * (task->width * task->channels) + i * task->channels;
        r += task->pixels[dx + 0];
        g += task->pixels[dx + 1];
        b += task->pixels[dx + 2];
      }
    }
    uint32_t elements = task->width * task->height;
    r /= elements;
    g /= elements;
    b /= elements;

    Descriptor desc;
    desc.filename = filename;
    desc.average_color[0] = r;
    desc.average_color[1] = g;
    desc.average_color[2] = b;

    descriptors.push_back(desc);

    printf("R: %llu G: %llu B: %llu\n", r,g,b);

    if (on_analyzed) {
      on_analyzed(desc, user);
    }

    return 0;
  }

  int AnalyzerCPU::saveDescriptors() {
    std::string outfile = rx_to_data_path("descriptors.txt");
    return save_descriptors(outfile, descriptors);
  }

  int AnalyzerCPU::loadDescriptors() {

    /* first clear the current descriptors */
    descriptors.clear();

    std::string outfile = rx_to_data_path("descriptors.txt");
    return load_descriptors(outfile, descriptors);
  }

  /* ---------------------------------------------------------------------------------- */

  static void* analyzer_cpu_thread(void* user) {

    /* get the analyzer */
    AnalyzerCPU* ana = static_cast<AnalyzerCPU*>(user);
    if (NULL == ana) {
      RX_ERROR("The user data in the analyzer thread is NULL. Not supposed to happen.");
      exit(1);
    }

    std::vector<AnalyzerTask*> todo;

    while (false == ana->must_stop) {

      /* wait for work to arrive */
      ana->lock();
      {
        while (0 == todo.size() && false == ana->must_stop) {
          pthread_cond_wait(&ana->cond, &ana->mutex);
          std::copy(ana->work.begin(), ana->work.end(), std::back_inserter(todo));
          ana->work.clear();
        }
      }
      ana->unlock();
      
      /* must stop? */
      if (ana->must_stop) {
        break;
      }

      if (0 == todo.size()) {
        /* spurious wake-up */
        continue;
      }

      RX_VERBOSE("Got %lu tasks", todo.size());

      for (size_t i = 0; i < todo.size(); ++i) {
        AnalyzerTask* task = todo[i];

        if (task->type == ANA_TASK_ANALYZE) {
          ana->executeAnalyzeTask(task);
        }
        else {
          RX_ERROR("Unhandled task: %d", task->type);
        }
        /* @todo -> free the task again -- and check if we actually should add a lock here? */
        task->reset();

        RX_VERBOSE("And free'd again.");
      }
      todo.clear();
    }

    ana->is_running = false;

    return NULL;
  }

} /* namespace fex */


