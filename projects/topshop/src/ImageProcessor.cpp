#include <topshop/ImageProcessor.h>
#include <featurex/Config.h>
#include <topshop/Config.h>

namespace top {

  /* ----------------------------------------------------------------------- */

  static void* image_processor_thread(void* user);
  static int image_processor_process_file(ImageProcessor* proc, CollectedFile& file);
  static void free_image_processor_tasks(std::vector<ProcessTask*>& tasks);

  /* ----------------------------------------------------------------------- */

  ImageProcessor::ImageProcessor() 
    :is_running(false)
    ,must_stop(true)
  {
    int r = 0;

    r = pthread_mutex_init(&mutex, NULL);
    if (0 != r) {
      RX_ERROR("Cannot init the mutex: %s", strerror(r));
      exit(0);
    }

    r = pthread_cond_init(&cond, NULL);
    if (0 != r) {
      RX_ERROR("Cannot init the condvar. :%s", strerror(r));
      exit(0);
    }
  }

  ImageProcessor::~ImageProcessor() {
    int r = 0;

    if (true == is_running) {
      shutdown();
    }

    r = pthread_mutex_destroy(&mutex);
    if (0 != r) {
      RX_ERROR("Failed to destroy the mutex: %s", strerror(r));
    }

    r = pthread_cond_destroy(&cond);
    if (0 != r) {
      RX_ERROR("Failed to destroy the cond var: %s", strerror(r));
    }
  }

  int ImageProcessor::init() {

    int r = 0;

    if (true == is_running) {
      RX_ERROR("Already running");
      return -1;
    }

    is_running = true;
    must_stop = false;

    r = pthread_create(&thread, NULL, image_processor_thread, this);

    if (0 != r) {
      RX_ERROR("Cannot create the image processor thread.");
      is_running = false;
      must_stop = true;
      return -2;
    }

    return 0;
  }

  int ImageProcessor::shutdown() {

    if (false == is_running) {
      RX_VERBOSE("ImageProcessor thread not running; ignoring shutdown request.");
      return 0;
    }
    
    /* tell the thread to stop */
    must_stop = true;
    lock();
      pthread_cond_signal(&cond);
    unlock();

    RX_VERBOSE("Joining image processor thread.");
    pthread_join(thread, NULL);

    lock();
      free_image_processor_tasks(tasks);
    unlock();

    if (0 != tasks.size()) {
      RX_ERROR("Tasks should have been empty now.");
    }

    is_running = false;

    return 0;
  }

  int ImageProcessor::process(CollectedFile file) {

    ProcessTask* task = new ProcessTask();
    if (NULL == task) {
      RX_ERROR("Cannot allocate a new process task; out of memory?");
      return -1;
    }

    RX_VERBOSE("Adding a file to the process queue: %s", file.filename.c_str());

    task->file = file;

    lock();
    {
      tasks.push_back(task);
      pthread_cond_signal(&cond);
    }
    unlock();

    return 0;
  }

  /* ----------------------------------------------------------------------- */

  static void* image_processor_thread(void* user) {

    ImageProcessor* proc = static_cast<ImageProcessor*>(user);
    if (NULL == proc) {
      RX_ERROR("Invalid user pointer in the image processor thread.");
      exit(EXIT_FAILURE);
    }

    std::vector<ProcessTask*> todo;

    while (false == proc->must_stop) {

      /* get new tasks. */
      proc->lock();
      {
        while (0 == proc->tasks.size() && false == proc->must_stop) {
          pthread_cond_wait(&proc->cond, &proc->mutex);
        }
        std::copy(proc->tasks.begin(), proc->tasks.end(), std::back_inserter(todo));
        proc->tasks.clear();
      }
      proc->unlock();

      /* do we need to stop? */
      if (proc->must_stop) {
        RX_VERBOSE("Stopping image processor thread.");
        free_image_processor_tasks(todo);
        return NULL;
      }

      if (0 == todo.size()) {
        /* spurious wake up */
        continue;
      }

      /* do the work. */
      for (size_t i = 0; i < todo.size(); ++i) {
        CollectedFile& file = todo[i]->file;
        // RX_VERBOSE("Processing: %s", file.filename.c_str());
        if ( 0 != image_processor_process_file(proc, file)) {
          RX_ERROR("Processing went wrong; see above log.");
        }
      }

      /* cleanup */
      free_image_processor_tasks(todo);
      if (0 != todo.size()) {
        RX_ERROR("free_image_processor_tasks failed to clear!");
      }
    }

    return NULL;
  }

  /* free the given tasks. */
  static void free_image_processor_tasks(std::vector<ProcessTask*>& tasks) {
    for (size_t i = 0; i < tasks.size(); ++i) {
      delete tasks[i];
      tasks[i] = NULL;
    }
    tasks.clear();
  }

  static int image_processor_process_file(ImageProcessor* proc, CollectedFile& file) {

    std::string filepath = file.dir +"/" +file.filename;

    if (NULL == proc) {
      RX_ERROR("The ImageProcessor handle is invalid.");
      return -1;
    }

    /* create the polaroid images. */
    std::string json_filepath = top::config.json_filepath +"/" +rx_strip_file_ext(file.filename) +".json";
    if (false == rx_file_exists(json_filepath)) {
      RX_ERROR("Cannot find the json for %s", json_filepath.c_str());
      return -5;
    }
    
    ImageInfo img_info;
    if (0 != proc->img_json.parse(json_filepath, img_info)) {
      RX_ERROR("Failed to parse json: %s", json_filepath.c_str());
      return -6;
    }

    RX_VERBOSE("Username: %s", img_info.username.c_str());
    if (0 == img_info.username.size()) {
      RX_ERROR("The username is empty, not supposed to happen.");
      img_info.username = "anonymous";
    }

    /* convert the username to uppercase. */
    std::string username_upper = img_info.username;
    std::transform(username_upper.begin(), username_upper.end(), username_upper.begin(), toupper);

    switch (file.type) {
      case COL_FILE_TYPE_NONE: {
        RX_ERROR("The collected file type isn't set!");
        return -4;
      }
      case COL_FILE_TYPE_LEFT_GRID: { 

        std::stringstream ss;
        ss << rx_get_exe_path() +"/scripts/preprocess_left_grid.sh " << filepath.c_str() 
           << " " << top::config.grid_file_width 
           << " " << top::config.grid_file_height 
           << " " << top::config.left_grid_filepath        /* this is where the files are stored. */
           << " " << fex::config.raw_filepath              /* images from the left grid are added to the mosaic after preprocessing */
           << " " << top::config.polaroid_filepath         /* path where the big versions of the polaroids are stored.*/
           << " " << username_upper                        /* username, used to create polaroid. */
           << "";

        std::string command = ss.str();
        system(command.c_str());
        return 0;
      }

      case COL_FILE_TYPE_RIGHT_GRID: {

        std::stringstream ss;
        ss << rx_get_exe_path() +"/scripts/preprocess_right_grid.sh " << filepath.c_str() 
           << " " << top::config.grid_file_width 
           << " " << top::config.grid_file_height 
           << " " << top::config.right_grid_filepath        /* this is where the files are stored. */
           << " " << fex::config.raw_filepath               /* images from the left grid are added to the mosaic after preprocessing */
           << " " << top::config.polaroid_filepath          /* path where the big versions of the polaroids are stored.*/
           << " " << username_upper                         /* username, used to create polaroid. */
           << "";

        std::string command = ss.str();
        system(command.c_str());
        return 0;
      }

      default: {
        RX_ERROR("Unhandled collected file type: %d", file.type);
        return -1;
      }
    }

    return 0;
  } 

} /* namespace top */
