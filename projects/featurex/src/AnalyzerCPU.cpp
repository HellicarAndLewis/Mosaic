#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <featurex/Config.h>
#include <featurex/AnalyzerCPU.h>

namespace fex {

  /* ---------------------------------------------------------------------------------- */

  static int convert_to_rgba_png(AnalyzerCPU* ana, AnalyzerTask* task, std::string filepath);
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
    ,rgba(NULL)
  {

    /* the mutex and cond var are kept for the whole lifetime of the analyzer */
    if (0 !=  pthread_mutex_init(&mutex, NULL)) {
      RX_ERROR("Cannot initialize the mutex.");
    }

    if (0 != pthread_cond_init(&cond, NULL)) {
      RX_ERROR("Cannot initilialize the cond var.");
    }
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

    /* and destory the mutex + cond var */
    int r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Cannot destroy mutex: %d", r);
    }

    r = pthread_cond_destroy(&cond);
    if (0 != r) {
      RX_ERROR("Cannot destory condiation variable: %d", r);
    }
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

    if (NULL != rgba) {
      RX_ERROR("It seems that the rgba buffer is already created. Did you call shutdown?");
      return -4;
    }

    /* create buffer that is used when converting input tile pixels to 4 channels */
    if (0 == fex::config.file_tile_width) {
      RX_ERROR("File tile width is 0");
      return -5;
    }
    if (0 == fex::config.file_tile_height) {
      RX_ERROR("File tile height is 0");
      return -6;
    }

    int tile_bytes = fex::config.file_tile_width * fex::config.file_tile_height * 4;
    rgba = (unsigned char*) malloc(tile_bytes);
    if (NULL == rgba) {
      RX_ERROR("Error while allocating the rgba buffer");
      return -7;
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

    for (size_t i = 0; i < tasks.size(); ++i) {
      delete tasks[i];
    }

    if (NULL != rgba) {
      free(rgba);
      rgba = NULL;
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

    int r = 0;

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
      r = pthread_cond_signal(&cond);
      if (0 != r) {
        RX_ERROR("Cannot signal the condition var: %d", r);
      }
    }
    unlock();

    RX_VERBOSE("Added %s to the CPU analyzer thread.", filepath.c_str());
  
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
          result->is_free = false;
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

      result->is_free = false;

      /* and append to the list */
      lock();
        tasks.push_back(result);
      unlock();
    }

    /* log a message; just a bit of extra info - this probably will never happen. */
    if (100 < num_tasks) {
      RX_WARNING("There are currently %lu tasks created; are we freeing them? Or do we get too many images?", num_tasks);
    }

    return result;
  }

  /*
    IMPORTANT: This is called from the thread and this is where we will
    analyze the images for the given task.
   */
  int AnalyzerCPU::executeAnalyzeTask(AnalyzerTask* task) {

#if !defined(NDEBUG)
    if (0 == fex::config.file_tile_width || 0 == fex::config.file_tile_height) {
      RX_ERROR("Either the file_tile_width or file_tile_height is invalid");
      ::exit(EXIT_FAILURE);
    }
#endif

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

    /* we're only using .png when analyzing; the preprocess.sh file converts the images to png. */
    std::string filename = rx_strip_file_ext(rx_strip_dir(task->filepath)) +".png";
    std::string basename = rx_strip_file_ext(filename);
    std::string resized_filepath = fex::config.resized_filepath +"/" +basename +".png";
    std::string exe_path = rx_get_exe_path();

    /* create the command line the preprocess task.*/
    std::stringstream ss;
    ss << exe_path +"/scripts/preprocess_mosaic.sh " << task->filepath 
       << " " << fex::config.file_tile_width 
       << " " << fex::config.file_tile_height
       << " " << fex::config.resized_filepath;

    std::string command = ss.str();
                                

    if (0 == filename.size()) {
      RX_ERROR("Filename is invalid.");
      return -3;
    }
    system(command.c_str());

    /* make sure the image we use has 4 channels. */
    if (0 != convert_to_rgba_png(this, task, resized_filepath)) {
      return -3;
    }

    /* check if the resized file was created. */
    if (false == rx_file_exists(resized_filepath)) {
      RX_ERROR("Cannot find the resized file: %s", resized_filepath.c_str());
      return -4;
    }

    /* make sure we have a png */
    ext = rx_get_file_ext(resized_filepath);
    if (ext == "jpg") {
      RX_ERROR("This is something we need to clean up. The resized images need to be all 4 channels - is necessary for the fast memcpy ");
      return -5;
    }

    /* laod the image */
    ret = rx_load_png(resized_filepath, &task->pixels, task->width, task->height, task->channels, &task->capacity);
    if (ret < 0) {
      RX_ERROR("Something went wrong with loading %s", resized_filepath.c_str());
      return -6;
    }

    RX_VERBOSE("Loaded %s, %d x %d, channels: %d, bytes allocated %d", 
               resized_filepath.c_str(),
               task->width,
               task->height,
               task->channels,
               task->capacity);

    if (3 == task->channels) {
      RX_WARNING("We're not analyzing the image because the number of channels is not yet supported: %d", task->channels);
      return -7;
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
    desc.setFilename(filename);
    desc.average_color[0] = r;
    desc.average_color[1] = g;
    desc.average_color[2] = b;

    descriptors.push_back(desc);

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

    int r = 0;
    std::string outfile;

    outfile  = rx_to_data_path("descriptors.txt");

    /* first clear the current descriptors then load them*/
    descriptors.clear();
    r =  load_descriptors(outfile, descriptors);
    if (0 != r) {
      return r;
    }

    /* make sure that we don't load too many */
    descriptors.resize(std::min<size_t>(fex::config.memory_pool_size, descriptors.size()));
    
    return 0;
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
        while (0 == ana->work.size() && false == ana->must_stop) {
          pthread_cond_wait(&ana->cond, &ana->mutex);
        }
        std::copy(ana->work.begin(), ana->work.end(), std::back_inserter(todo));
        ana->work.clear();
      }
      ana->unlock();
      
      /* must stop? */
      if (ana->must_stop) {
        RX_VERBOSE("Analyzer thread was asked to stop - shouldn't we free the todo tasks here?");
        break;
      }

      if (0 == todo.size()) {
        /* spurious wake-up */
        continue;
      }

      RX_VERBOSE("Processing %lu tasks", todo.size());
      for (size_t i = 0; i < todo.size(); ++i) {
        AnalyzerTask* task = todo[i];

        if (ana->must_stop) {
          RX_VERBOSE("Stopping with the analyze task");
          break;
        }

        RX_VERBOSE("Processing task nr. %lu/%lu", i, todo.size());

        if (task->type == ANA_TASK_ANALYZE) {
          ana->executeAnalyzeTask(task);
        }
        else {
          RX_ERROR("Unhandled task: %d", task->type);
        }
      }

      RX_VERBOSE("Ready, done %lu tasks", todo.size());

      /* and reset all tasks. */
      ana->lock();
      {
        for (size_t i = 0; i < todo.size(); ++i) {
          todo[i]->reset();
        }
      }
      ana->unlock();

      todo.clear();
    }

    ana->is_running = false;

    return NULL;
  }

  /* 
     converts the given filepath to a 4 channel png. we use the rgba buffer of the analyzer. 
     the input image must be a PNG file!
   */
  static int convert_to_rgba_png(AnalyzerCPU* ana, AnalyzerTask* task, std::string filepath) {
    std::string input_filepath = filepath;
    std::string ext = rx_get_file_ext(input_filepath);
    int nbytes;
    int i,j,k;
    int src_dx, dest_dx;

    if (NULL == task) {
      RX_ERROR("Invalid task");
      return -1;
    }

    /* check if the file exists .*/
    if (false == rx_file_exists(input_filepath)) {
      RX_ERROR("Cannot find the file that we need to convert to rgba: %s", input_filepath.c_str());
      return -2;
    }

    /* load the image. */
    if (ext != "png") {
      RX_ERROR("We don't support %s files.", ext.c_str());
      return -3;
    }

    /* load and check if the file has 4 channels, if so we're good. */
    nbytes = rx_load_png(input_filepath, &task->pixels, task->width, task->height, task->channels, &task->capacity);
    if (task->channels == 4) {
      return 0;
    }

    /* convert the pixels to 4 channels */
    for (i = 0; i < task->width; ++i) {
      for (j = 0; j < task->height; ++j) {
        dest_dx = j * task->width * 4 + i * 4;
        src_dx = j * task->width * task->channels + i * task->channels;
        for (k = 0; k < 4; ++k) {
          if (k < task->channels) {
            ana->rgba[dest_dx + k] = task->pixels[src_dx + k];
          }
          else {
            ana->rgba[dest_dx + k] = 0xFF;
          }
        }
      }
    }
     
    if (false == rx_save_png(filepath, ana->rgba, task->width, task->height, 4, false)) {
      RX_ERROR("Cannot save the PNG: %s with 4 channels", filepath.c_str());
      return -4;
    }

    return 0;
  }

} /* namespace fex */


