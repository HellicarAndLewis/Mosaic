/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------


  ImageCollector
  --------------

  The ImageCollector watches a couple of directories that are used by the 
  TopShop application to generate the mosaic and the left/right grids. We're 
  watching the following directories (see settings.xml for more info):
  
  raw_filepath: 
             This is the main directory. When files are added to this 
             directory they will be added to the CPU analyzer which analyzes
             the image in a separate thread and calls the process.sh shell 
             script that creates a couple of alternatives for the image file.


   @todo - add other directories we watch. 
            
   The collector acts as a bin into which file are added for the different
   screens of the installation. It makes sure that the flow of incoming images
   is somewhat regulated and not 1000s of imges are added at once. We use a delay
   for each of the collected file bins. 

*/

#ifndef TOPSHOP_IMAGE_COLLECTOR_H
#define TOPSHOP_IMAGE_COLLECTOR_H

#define ROXLU_USE_LOG
#include <tinylib.h>

#include <mosaic/DirWatcher.h>
#include <deque>


#define COL_FILE_TYPE_NONE 0x00
#define COL_FILE_TYPE_RAW 0x01
#define COL_FILE_TYPE_LEFT_GRID 0x02
#define COL_FILE_TYPE_RIGHT_GRID 0x03


namespace top {

  /* ------------------------------------------------------------------------- */
  class ImageCollector;
  class CollectedFile;
  typedef void (*image_collector_callback)(ImageCollector* col, CollectedFile& file);

  /* ------------------------------------------------------------------------- */

  class CollectedFile {
  public:
    CollectedFile();
    ~CollectedFile();
    void reset();

  public:
    int type; 
    std::string dir;
    std::string filename;
    uint64_t timestamp;
  };

  /* ------------------------------------------------------------------------- */

  class ImageCollector {
  public:
    ImageCollector();
    ~ImageCollector();
    int init(std::string filepath);
    void update();
    int shutdown();

  public:
    mos::DirWatcher dir_watcher;              /* watches the raw_filepath directory. */
    std::deque<CollectedFile> files;
    uint64_t timestamp;
    uint64_t delay;
    void* user;
    image_collector_callback on_file;
  };

} /* namespace top */


#endif
