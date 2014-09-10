/*
  
  AppImageProcessor
  -----------------

  The AppImageProcessor can be used to listen to the input/watch 
  directories of the left and right grids when you're not running 
  those. This application will listen to the left/right dirs and 
  will will process the files that are added. 

  This will make sure that the same files are created that would 
  have been created when using the AppGridLeft and AppGridRight
  applications.


*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define ROXLU_IMPLEMENTATION
#define ROXLU_USE_LOG
#define ROXLU_USE_JPG
#define ROXLU_USE_PNG
#include <tinylib.h>

#include <mosaic/Config.h>
#include <topshop/Config.h>
#include <topshop/ImageCollector.h>
#include <topshop/ImageProcessor.h>

/* -------------------------------------------------------------------------------- */

static void sighandler(int s);
static void on_new_file(top::ImageCollector* col, top::CollectedFile& file);

/* -------------------------------------------------------------------------------- */

class Application {
public:
  Application(int type);
  ~Application();
  int init(std::string path);
  void update();
  int shutdown();

public:
  top::ImageCollector img_collector;
  top::ImageProcessor img_processor;
  int file_type;
  int is_init;
};

/* -------------------------------------------------------------------------------- */

Application::Application(int type) 
  :file_type(type)
  ,is_init(-1)
{
}

Application::~Application() {
  file_type = COL_FILE_TYPE_NONE;
  is_init = -1;
}

int Application::init(std::string path) {

  int r;

  if (0 == path.size()) {
    RX_ERROR("Path is empty.");
    return -1;
  }

  if (COL_FILE_TYPE_LEFT_GRID != file_type && COL_FILE_TYPE_RIGHT_GRID != file_type) {
    RX_ERROR("Unsupported file type: %d", file_type);
    return -2;
  }

  if (false == rx_is_dir(path)) {
    RX_ERROR("Filepath doesn't exist: %s", path.c_str());
    return -3;
  }

  r = img_processor.init();
  if (0 != r) {
    RX_ERROR("Failed to init the image processor.");
    return -4;
  }

  r = img_collector.init(path);
  if (0 != r) {
    RX_ERROR("Couldn't initialize the left collector: %d", r);
    return -5;
  }

  img_collector.on_file = on_new_file;
  img_collector.user = this;
  is_init = 1;

  r = img_collector.scandir();
  if (0 != r) {
    RX_ERROR("scandir failed. not a biggy, not supposed to happen.");
  }

  return 0;
}

void Application::update() {

  if (1 != is_init) {
    RX_ERROR("Application not initialized.");
    return;
  }

  img_collector.update();
}

int Application::shutdown() {

  RX_VERBOSE("Shutting down.");

  if (1 != is_init) {
    RX_ERROR("Application not initialized");
    return -1;
  }

  if (0 != img_collector.shutdown()) {
    RX_ERROR("Failed to shutdown the image collected cleanly.");
  }

  if (0 != img_processor.shutdown()) {
    RX_ERROR("Failed to shutdown the image processor.");
  }

  return 0;
}

/* -------------------------------------------------------------------------------- */

bool must_stop = false;
Application app_left(COL_FILE_TYPE_LEFT_GRID); /* listens to the dir of the left grid */
Application app_right(COL_FILE_TYPE_RIGHT_GRID); /* listens to the dir fo the right grid. */

/* -------------------------------------------------------------------------------- */

int main() {

  int r;

  /* make sure the logpath exists. */
  std::string logpath = rx_to_data_path("log_image_processor");
  if (false == rx_is_dir(logpath)) {
    printf("\n\nError: %s doesn't exit, create it.\n\n", logpath.c_str());
    exit(EXIT_FAILURE);
  }

  printf("\n\nAppImageProcessor\n\n");

  signal(SIGINT, sighandler);

  /* init log */
  rx_log_init(logpath);

  /* load + validate the configuration */
  if (0 != top::load_config()) {
    RX_ERROR("Cannot load the configuration.");
    exit(EXIT_FAILURE);
  }

  rx_log_set_level(top::config.log_level);

  if (0 != top::config.validate()) {
    RX_ERROR("Error while validating the config.");
    exit(EXIT_FAILURE);
  }

  if (0 != app_left.init(top::config.raw_left_grid_filepath)) {
    RX_ERROR("Cannot init the left app.");
    exit(EXIT_FAILURE);
  }

  if (0 != app_right.init(top::config.raw_right_grid_filepath)) {
    RX_ERROR("Cannot init the right app.");
    exit(EXIT_FAILURE);
  }

  while (false == must_stop) {
    app_left.update();
    app_right.update();
    usleep(1e6); /* wait Xe6 seconds */
  }

  app_left.shutdown();
  app_right.update();
  
  return 0;
}

/* -------------------------------------------------------------------------------- */

static void on_new_file(top::ImageCollector* col, top::CollectedFile& file) {

  if (NULL == col) {
    RX_ERROR("Invalid ImageCollector ptr.");
    return;
  }

  std::string ext = rx_get_file_ext(file.filename);
  if (ext != "jpg" && ext != "png") {
    RX_VERBOSE("Ignoring non image file: %s", file.filename.c_str());
    return;
  }

  Application* app = static_cast<Application*>(col->user);
  if (NULL == app) {
    RX_ERROR("Cannot get a handle to the App. not supposed to happen.");
    return;
  }
    
  std::string filepath = file.dir +"/" +file.filename;
  RX_VERBOSE("%s\n", file.filename.c_str());

  if (false == rx_file_exists(filepath)) {
    RX_ERROR("Filepath doesn't exists: %s", filepath.c_str());
    return;
  }

  file.type = app->file_type;

  /*
  RX_ERROR("GOT FILETYPE: %d, COL_FILE_TYPE_LEFT_GRID: %d, COL_FILE_TYPE_RIGHT_GRID: %d",
           file.type,
           COL_FILE_TYPE_LEFT_GRID,
           COL_FILE_TYPE_RIGHT_GRID);
  */

  if (0 != app->img_processor.process(file)) {
    RX_ERROR("Something went wrong in the image processor; see log above.");
  }
}

static void sighandler(int s) {
  RX_VERBOSE("Got signal.");
  must_stop = true;
}
