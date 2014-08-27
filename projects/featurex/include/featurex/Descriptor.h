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
#ifndef FEATUREX_DESCRIPTOR_H
#define FEATUREX_DESCRIPTOR_H

#include <stdint.h>
#include <string>
#include <vector>

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace fex { 

  /* ---------------------------------------------------------------------------------- */

  class Descriptor;

  int load_descriptors(std::string filename, std::vector<Descriptor>& descriptors);
  int save_descriptors(std::string filename, std::vector<Descriptor>& descriptors);

  /* ---------------------------------------------------------------------------------- */

  /* @todo - 32bit align the descriptor */
  class Descriptor {

  public:
    Descriptor();
    ~Descriptor();
    void reset();
    void setFilename(std::string filename);
    std::string& getFilename();

  public:
    uint32_t id;                                     /* unique ID based on the filename */
    uint32_t average_color[3];
    int row;                                         /* only used when constructing a mosaic; this will be set to the row position of the tile. */
    int col;                                         /* only used when constructing a mosaic; this will be set to the col position of the tile. */

  private:
    std::string filename;
  };

  /* ---------------------------------------------------------------------------------- */

  inline void Descriptor::setFilename(std::string fname) {
    if (0 == fname.size()) {
       RX_ERROR("Invalid filename; empty");
       return;
    }

    id = rx_string_id(fname);
    filename = fname;
    RX_VERBOSE("Descriptor: %llu", id);
  }

  inline std::string& Descriptor::getFilename() {
    return filename;
  }

  /* ---------------------------------------------------------------------------------- */

} /* namespace fex */
#endif
