#include <mosaic/DirWatcher.h>

namespace mos {

  /* --------------------------------------------------------------------- */

  static void on_dir_change(uv_fs_event_t* handle, const char* fname, int events, int status);

  /* --------------------------------------------------------------------- */

  DirWatcher::DirWatcher() 
    :loop(NULL)
    ,user(NULL)
    ,on_rename(NULL)
  {
  }

  DirWatcher::~DirWatcher() {
    if (NULL != loop) {
      shutdown();
    }
  }

  int DirWatcher::init(std::string dir, dirwatch_on_rename_callback cb, void* usr) {

    int r = 0;
    directory = dir;

    if (NULL != loop) {
      RX_ERROR("Looks like we're already initialize, call shutdown() first.");
      return -1;
    }

    if (false == rx_is_dir(directory)) {
      RX_ERROR("Not a directory: %s", directory.c_str());
      return -2;
    }

    loop = uv_default_loop();

    r = uv_fs_event_init(loop, &fs_event);
    if (0 != r) {
      RX_ERROR("Cannot initialize the fs event: %s", uv_strerror(r));
      loop = NULL;
      return -3;
    }

    r = uv_fs_event_start(&fs_event, on_dir_change, directory.c_str(), 0);
    if (0 != r) {
      RX_ERROR("Cannot start the fs event: %s", uv_strerror(r));
      loop = NULL;
      return -4;
    }

    fs_event.data = this;
    on_rename = cb;
    user = usr;

    return 0;
  }

  /* scans the watch dir, just call this ones to process files which for which no event was given */
  int DirWatcher::scandir() {

    if (0 == directory.size()) {
      RX_ERROR("Directory is not set, cannot scan the dir.");
      return -1;
    }

    if (NULL == on_rename) {
      RX_ERROR("Not sanning director: %s because the on_rename callback is not set.", directory.c_str());
      return -2;
    }
    
    if (false == rx_is_dir(directory)) {
      RX_ERROR("Cannot scan the directory because it's not a dir: %s", directory.c_str());
      return -3;
    }

    std::vector<std::string> files = rx_get_files(directory, "*");    
    for (size_t i = 0; i < files.size(); ++i) {
      on_rename(directory, rx_strip_dir(files[i]), user);
    }

    return 0;
  }

  void DirWatcher::update() {
    if (NULL != loop) {
      uv_run(loop, UV_RUN_NOWAIT);
    }
  }

  int DirWatcher::shutdown() {
    int r = 0;

    if (NULL == loop) {
      RX_WARNING("Asking to shutdown, but loop is NULL. Did you call init()?");
      return -1;
    }

    /* stop listening for dir changes.*/
    r = uv_fs_event_stop(&fs_event);
    if (0 != r) {
      RX_ERROR("Error while trying to stop the fs_event: %s", uv_strerror(r));
    }

    directory.clear();

    loop = NULL;
    user = NULL;
    on_rename = NULL;

    return 0;
  }

  /* --------------------------------------------------------------------- */

  static void on_dir_change(uv_fs_event_t* handle, const char* fname, int events, int status) {
    if (UV_RENAME != events) {
      return ;
    }
    
    DirWatcher* watch = static_cast<DirWatcher*>(handle->data);
    if (NULL == watch) {
      RX_ERROR("No data member set on the fs_event handle, not supposed to happend!");
      return;
    }

    /* call callback */
    if (NULL == watch->on_rename) {
      RX_WARNING("Got an dir change event but no on_rename callback set.");
      return;
    }

    /* we get a UV_RENAME event when an image is added and/or removed, we need to figure out what it was */
    if (false == rx_file_exists(watch->directory +"/" +fname)) {
      return;
    }

    watch->on_rename(watch->directory, fname, watch->user);
  }

  /* --------------------------------------------------------------------- */
  
} /* namespace mos */
