#ifndef TOPSHOP_GRID_APP_SIMPLE_H
#define TOPSHOP_GRID_APP_SIMPLE_H

#include <grid/SimpleGrid.h>
#include <mosaic/DirWatcher.h>
#include <topshop/ImageCollector.h>
#include <topshop/ImageProcessor.h>

namespace top {

  class GridAppSimple {
  public:
    GridAppSimple();
    ~GridAppSimple();
    int init(grid::SimpleSettings cfg);
    void update(); 
    void draw();
    int shutdown();

  private:
    int scandir();                                              /* scans the cfg.image_dir, sorts on mtime and will start by filling the grid with these images. */
    
  public:
    mos::DirWatcher raw_watcher;
    mos::DirWatcher img_watcher;
    top::ImageProcessor img_processor;
    grid::SimpleGrid grid;
    grid::SimpleSettings settings;
    std::deque<grid::SimpleImage> images;                       /* we keep track of 2 sets of images that we use to flip when no new images are added to the system */
    size_t max_files;                                           /* cols * rows * 2 */
    uint64_t auto_change_timeout;                               /* when no images are added for a long period, we will automatically change them after some time */         
    uint64_t auto_change_delay;
  };

} /* namespace top */

#endif
