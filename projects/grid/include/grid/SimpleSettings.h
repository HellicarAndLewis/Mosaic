#ifndef ROXLU_SIMPLE_SETTINGS_H
#define ROXLU_SIMPLE_SETTINGS_H

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace grid {
  
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
  };
} /* namespace grid */

#endif
