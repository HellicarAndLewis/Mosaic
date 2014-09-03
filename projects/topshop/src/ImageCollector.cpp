#include <topshop/ImageCollector.h>
#include <featurex/Config.h>
#include <topshop/Config.h>
#include <sys/time.h>

namespace top { 

  /* ------------------------------------------------------------------------- */
  static ImageCollector* validate_new_file(std::string dir, std::string filename, void* user);    /* validates the input in the dir watcher callback(s) */
  static void raw_dir_on_rename(std::string dir, std::string filename, void* user);               /* gets called when a new file is stored in the raw filepath */
  static void raw_left_dir_on_rename(std::string dir, std::string filename, void* user);          /* gets called when a new file is stored in the raw left grid filepath */
  static void raw_right_dir_on_rename(std::string dir, std::string filename, void* user);         /* gets called when a new file is stored in the raw right grid filepath */
  static void limit_collected_files(std::deque<CollectedFile>& files, size_t maxfiles);           /* will make sure that the lists with files will not become to big; it will remote the oldest when the list is full */
  /* ------------------------------------------------------------------------- */
  
  CollectedFile::CollectedFile() {
    reset();
  }

  CollectedFile::~CollectedFile() {
    reset();
  }

  void CollectedFile::reset() {
    type = COL_FILE_TYPE_NONE;
    dir.clear();
    filename.clear();
    timestamp = 0;
  }

  /* ------------------------------------------------------------------------- */

  ImageCollector::ImageCollector() 
    :user(NULL)
    ,on_raw_file(NULL)
    ,on_left_grid_file(NULL)
    ,on_right_grid_file(NULL)
    ,raw_timestamp(0)
    ,left_grid_timestamp(0)
    ,right_grid_timestamp(0)
    ,raw_delay(300e6)
    ,left_grid_delay(300e6)
    ,right_grid_delay(300e6)
  {
  }

  ImageCollector::~ImageCollector() {
    user = NULL;
    on_raw_file = NULL;
    on_left_grid_file = NULL;
    on_right_grid_file = NULL;

    raw_files.clear();
    left_grid_files.clear();
    right_grid_files.clear();

    raw_timestamp = 0;
    left_grid_timestamp = 0;
    right_grid_timestamp = 0;
    
    raw_delay = 0;
    left_grid_delay = 0;
    right_grid_delay = 0;
  }

  int ImageCollector::init() {

    if (0 != raw_dir_watcher.init(fex::config.raw_filepath, raw_dir_on_rename, this)) {
      RX_ERROR("Cannot init the raw dir watcher.");
      return -1;
    }
    
    if (0 != left_dir_watcher.init(top::config.raw_left_grid_filepath, raw_left_dir_on_rename, this)) {
      RX_ERROR("Cannot init the left grid dir watcher.");
      return -2;
    }

    if (0 != right_dir_watcher.init(top::config.raw_right_grid_filepath, raw_right_dir_on_rename, this)) {
      RX_ERROR("Cannot init the rght grid dir watcher.");
      return -2;
    }
    

    return 0;
  }
  
  void ImageCollector::update() {

#if !defined(NDEBUG)
    if (NULL == on_raw_file)        {  RX_ERROR("No on_raw_file callback set.");          ::exit(EXIT_FAILURE);  }
    if (NULL == on_left_grid_file)  {  RX_ERROR("No on_left_grid_file callback set.");    ::exit(EXIT_FAILURE);  }
    if (NULL == on_right_grid_file) {  RX_ERROR("No on_right_grid_file callback set.");   ::exit(EXIT_FAILURE);  }
#endif

    raw_dir_watcher.update();
    left_dir_watcher.update();
    right_dir_watcher.update();
    
    uint64_t now = rx_hrtime();

    /* check if we have raw files. */
    if (now >= raw_timestamp && NULL != on_raw_file) {
      raw_timestamp = now + raw_delay;
      if (0 != raw_files.size()) {
        CollectedFile cfile = raw_files.front();
        on_raw_file(this, cfile);
        raw_files.pop_front();
      }
    }

    /* check if we have new left grid files. */
    if (now >= left_grid_timestamp && NULL != on_left_grid_file) {
      left_grid_timestamp = now + left_grid_delay;
      if (0 != left_grid_files.size()) {
        CollectedFile cfile = left_grid_files.front();
        on_left_grid_file(this, cfile);
        left_grid_files.pop_front();
      }
    }

    /* check if we have new right grid files. */
    if (now >= right_grid_timestamp && NULL != on_right_grid_file) {
      right_grid_timestamp = now + right_grid_delay;
      if (0 != right_grid_files.size()) {
        CollectedFile cfile = right_grid_files.front();
        on_right_grid_file(this, cfile);
        right_grid_files.pop_front();
      }
    }

    /* make sure the list with files will not become too large. */
    size_t max_files = 5000;
    limit_collected_files(raw_files, max_files);
    limit_collected_files(left_grid_files, max_files);
    limit_collected_files(right_grid_files, max_files);
  }

  int ImageCollector::shutdown() {
    int r = 0;

    r = raw_dir_watcher.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to cleanly shutdown the raw dir watcher: %d", r);
    }
    
    r = left_dir_watcher.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to cleanly shutdown the left dir watcher: %d", r);
    }

    r = right_dir_watcher.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to cleanly shutdown the right dir watcher: %d", r);
    }

    raw_files.clear();
    left_grid_files.clear();
    right_grid_files.clear();

    raw_timestamp = 0;
    left_grid_timestamp = 0;
    right_grid_timestamp = 0;
    
    raw_delay = 0;
    left_grid_delay = 0;
    right_grid_delay = 0;

    return 0;
  }

  /* ------------------------------------------------------------------------- */

  static void raw_dir_on_rename(std::string dir, std::string filename, void* user) {

    /* validate + get the handle to our image collector. */
    ImageCollector* collector = validate_new_file(dir, filename, user);
    if (NULL == collector) {
      return;
    }
    
    CollectedFile cfile;
    cfile.type = COL_FILE_TYPE_RAW;
    cfile.dir = dir;
    cfile.filename = filename;
    cfile.timestamp = rx_hrtime();

    collector->raw_files.push_back(cfile);
  }

  /* add a new file for the left grid */
  static void raw_left_dir_on_rename(std::string dir, std::string filename, void* user) {
    ImageCollector* collector = validate_new_file(dir, filename, user);
    if (NULL == collector) { return;  }
    CollectedFile cfile;
    cfile.type = COL_FILE_TYPE_LEFT_GRID;
    cfile.dir = dir;
    cfile.filename = filename;
    cfile.timestamp = rx_hrtime();
    collector->left_grid_files.push_back(cfile);
  }

  /* add a new file for the right grid */
  static void raw_right_dir_on_rename(std::string dir, std::string filename, void* user) {
    ImageCollector* collector = validate_new_file(dir, filename, user);
    if (NULL == collector) { return;  }
    CollectedFile cfile;
    cfile.type = COL_FILE_TYPE_RIGHT_GRID;
    cfile.dir = dir;
    cfile.filename = filename;
    cfile.timestamp = rx_hrtime();
    collector->right_grid_files.push_back(cfile);
  }

  /* validates the given parameters and returns the ImageCollector* that we cast from a event handler */
  static ImageCollector* validate_new_file(std::string dir, std::string filename, void* user) {
    if (NULL == user) {
      RX_ERROR("User pointer is NULL. Not supposed to happen.");
      return NULL;
    }

    ImageCollector* collector = static_cast<ImageCollector*>(user);
    if (NULL == collector) {
      RX_ERROR("Cannot cast user pointer to the ImageCollector.");
      return NULL;
    }

    if (0 == dir.size()) {
      RX_ERROR("Got new file event but the dir value is empty. Not supposed to happen.");
      return NULL;
    }

    if (0 == filename.size()) {
      RX_ERROR("Got a new file event but the filename is empty. Not supposed to happen.");
      return NULL;
    }

    std::string filepath = dir +"/" +filename;
    if (false == rx_file_exists(filepath)) {
      RX_ERROR("The file %s doesn't exist.", filepath.c_str());
      return NULL;
    }

    return collector;

  }

  /* makes sure the given vector will not have too many items. */
  void limit_collected_files(std::deque<CollectedFile>& files, size_t maxfiles) {
    while (files.size() > maxfiles) {
      RX_VERBOSE("Removing!");
      files.pop_front();
    }
  }
  /* ------------------------------------------------------------------------- */

} /* namespace top */
