#include <topshop/GridAppSimple.h>

namespace top {

  /* ------------------------------------------------------------------------- */
  static void on_raw_dir_file(std::string dir, std::string filename, void* user);
  static void on_image_dir_file(std::string dir, std::string filename, void* user);
  static int validate_file(std::string dir, std::string filename, void* user);
  static void on_grid_event(grid::SimpleGrid* grid, grid::SimpleLayer* layer, int event);
  /* ------------------------------------------------------------------------- */

  GridAppSimple::GridAppSimple() 
    :max_files(0)
    ,auto_change_timeout(0)
    ,auto_change_delay(6e9)
  {
  }

  GridAppSimple::~GridAppSimple() {
    shutdown();
  }

  int GridAppSimple::init(grid::SimpleSettings cfg) {

    int r = 0;
    
    if (false == cfg.validate()) {
      return -1;
    }

    settings = cfg;

    r = grid.init(cfg);
    if (0 != r) {
      RX_ERROR("Cannot init the grid.");
      return r;
    }

    r = raw_watcher.init(cfg.watch_dir, on_raw_dir_file, this);
    if (0 != r) {
      RX_ERROR("Cannot init the raw dir watcher.");
      grid.shutdown();
      return -2;
    }

    r = img_watcher.init(cfg.image_dir, on_image_dir_file, this);
    if (0 != r) {
      RX_ERROR("Cannot init image dir watcher.");
      raw_watcher.shutdown();
      grid.shutdown();
      return -3;
    }

    r = img_processor.init();
    if (0 != r) {
      RX_ERROR("Cannot start image processor.");
      grid.shutdown();
      raw_watcher.shutdown();
      img_watcher.shutdown();
      return r;
    }

    max_files = cfg.cols * cfg.rows * 2; /* we keep track of 2 sets so we can switch when not enough files come in the system. */

    grid.on_event = on_grid_event;
    grid.user = this;

    /* process files which are still in the watch dir or already procesed. */
    img_watcher.scandir();
    raw_watcher.scandir();



    return 0;
  }

  void GridAppSimple::update() {

    /* check if we need to change the grid contents because it took too long for new files to come in */
    uint64_t now = rx_hrtime();
    if (0 != auto_change_timeout && now > auto_change_timeout) {
      auto_change_timeout = now + auto_change_delay;
      size_t files_to_add = settings.cols * settings.rows;
      for (size_t i = 0; i < files_to_add; ++i) {
        grid::SimpleImage& img = images.front();
        grid.addImage(img);
        images.pop_front();
        images.push_back(img);
      }
    }

    raw_watcher.update();
    img_watcher.update();

    grid.updatePhysics(0.16f);
    grid.update();
  }

  void GridAppSimple::draw() {
    grid.draw();
  }

  int GridAppSimple::shutdown() {
    raw_watcher.shutdown();
    img_watcher.shutdown();
    img_processor.shutdown();
    grid.shutdown();
    return 0;
  }

  /* ------------------------------------------------------------------------- */

  static void on_raw_dir_file(std::string dir, std::string filename, void* user) {

    if (0 != validate_file(dir, filename, user)) {
      return;
    }

    GridAppSimple* app = static_cast<GridAppSimple*>(user);
    if (NULL == app) {
      RX_ERROR("Cannot get app");
      return;
    }

    RX_VERBOSE("Got raw file: %s", filename.c_str());

    CollectedFile col_file;
    col_file.dir = dir;
    col_file.filename = filename;

    if (grid::SIMPLE_GRID_DIRECTION_RIGHT == app->grid.settings.direction) {
      col_file.type = COL_FILE_TYPE_LEFT_GRID;
    }
    else if (grid::SIMPLE_GRID_DIRECTION_LEFT == app->grid.settings.direction) {
      col_file.type = COL_FILE_TYPE_RIGHT_GRID;
    }
    else {
      RX_ERROR("Unhanded grid direction.Ignoring file: %s, now: %f", col_file.filename.c_str(), rx_millis());
      return;
    }

    if (0 != app->img_processor.process(col_file)) {
      RX_ERROR("Something went wrong in the image processor; see log above.");
    }
  }

  static void on_image_dir_file(std::string dir, std::string filename, void* user) {

    if (0 != validate_file(dir, filename, user)) {
      return;
    }

    GridAppSimple* app = static_cast<GridAppSimple*>(user);
    if (NULL == app) {
      RX_ERROR("Cannot get app");
      return;
    }

    RX_VERBOSE("Got image file: %s", filename.c_str());

    grid::SimpleImage img;
    img.filepath = dir +"/" +filename;
    app->images.push_back(img);

    /* make sure that we don't keep too many images */
    while (app->images.size() > app->max_files) {
      app->images.pop_front();
    }

    app->grid.addImage(img);
  }

  static int validate_file(std::string dir, std::string filename, void* user) {

    if (0 == dir.size()) {
      RX_ERROR("Dir size is 0");
      return -1;
    }

    if (0 == filename.size()) {
      RX_ERROR("Filename size is 0");
      return -2;
    }

    if (NULL == user) {
      RX_ERROR("User is NULL");
      return -3;
    }

    std::string filepath = dir + "/" + filename;
    if (false == rx_file_exists(filepath)) {
      RX_ERROR("Cannot find file: %s", filepath.c_str());
      return -4;
    }

    return 0;
  }

  static void on_grid_event(grid::SimpleGrid* grid, grid::SimpleLayer* layer, int event) {

    GridAppSimple* app = static_cast<GridAppSimple*>(grid->user);
    if (NULL == app) {
      RX_ERROR("Cannot cast to GridAppSimple");
      return;
    }

    /* when we get a FULL event we update our change automatic change timeout */
    if (grid::SIMPLE_EVENT_SHOWN == event) {
      app->auto_change_timeout = rx_hrtime() + app->auto_change_delay;
      RX_VERBOSE("Everything is shown, updated auto change updated so we start again at: %llu, %f", app->auto_change_timeout, rx_millis());
    }

  }

} /* namespace top */
