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


*/
#ifndef ROXLU_SIMPLE_SETTINGS_H
#define ROXLU_SIMPLE_SETTINGS_H

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace grid {
  
  enum {
    SIMPLE_GRID_DIRECTION_NONE,
    SIMPLE_GRID_DIRECTION_LEFT,
    SIMPLE_GRID_DIRECTION_RIGHT
  };

  class SimpleSettings {
  public:
    SimpleSettings();
    ~SimpleSettings();
    bool validate();

  public:
    int rows;
    int cols;
    int img_width;
    int img_height;
    int padding_x;
    int padding_y;
    int offset_x;
    int offset_y;
    int direction;
    std::string watch_dir;  /* we watch this dir for new files */
    std::string image_dir;  /* we preload the last X images from this directory */ 
  };

} /* namespace grid */

#endif
