#ifndef FEATUREX_IMAGE_JSON_H
#define FEATUREX_IMAGE_JSON_H

#include <string>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace top {

  /* ------------------------------------------------- */

  struct ImageInfo {
    std::string username;
  };

  /* ------------------------------------------------- */

  class ImageJSON {

  public:
    ImageJSON();
    ~ImageJSON();
    int parse(std::string filepath, ImageInfo& result);
  };

} /* namespace top */


#endif
