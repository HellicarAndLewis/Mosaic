#include <topshop/ImageCollector.h>
#include <featurex/Config.h>
#include <topshop/Config.h>
#include <sys/time.h>

namespace top { 

  /* ------------------------------------------------------------------------- */
  static ImageCollector* validate_new_file(std::string dir, std::string filename, void* user);    /* validates the input in the dir watcher callback(s) */
  static void on_rename(std::string dir, std::string filename, void* user);                       /* gets called when a new file is stored in the raw filepath */
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
    ,on_file(NULL)
    ,timestamp(0)
    ,delay(300e6)
  {
  }

  ImageCollector::~ImageCollector() {
    user = NULL;
    on_file = NULL;
    files.clear();
    timestamp = 0;
    delay = 0;
  }

  int ImageCollector::init(std::string filepath) {

    if (0 != dir_watcher.init(filepath, on_rename, this)) {
      RX_ERROR("Cannot init the raw dir watcher.");
      return -1;
    }

    return 0;
  }
  
  void ImageCollector::update() {

#if !defined(NDEBUG)
    if (NULL == on_file) { 
      RX_ERROR("No on_raw_file callback set.");          
      ::exit(EXIT_FAILURE); 
    }
#endif

    if (NULL == on_rename) {
      RX_ERROR("on_rename not set. no need to update");
      return;
    }

    dir_watcher.update();
    
    uint64_t now = rx_hrtime();

    /* check if we have raw files. */
    if (now >= timestamp && NULL != on_file) {
      timestamp = now + delay;
      if (0 != files.size()) {
        CollectedFile cfile = files.front();
        on_file(this, cfile);
       files.pop_front();
      }
    }

    /* make sure the list with files will not become too large. */
    size_t max_files = 5000;
    limit_collected_files(files, max_files);
 
  }

  int ImageCollector::shutdown() {
    int r = 0;

    r = dir_watcher.shutdown();
    if (0 != r) {
      RX_ERROR("Failed to cleanly shutdown the raw dir watcher: %d", r);
    }
    
    files.clear();
    timestamp = 0;
     
    return 0;
  }

  /* ------------------------------------------------------------------------- */

  static void on_rename(std::string dir, std::string filename, void* user) {

    /* validate + get the handle to our image collector. */
    ImageCollector* collector = validate_new_file(dir, filename, user);
    if (NULL == collector) {
      return;
    }
    
    CollectedFile cfile;
    cfile.dir = dir;
    cfile.filename = filename;
    cfile.timestamp = rx_hrtime();

    collector->files.push_back(cfile);
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
