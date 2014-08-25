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

namespace fex { 

  class Descriptor;

  int load_descriptors(std::string filename, std::vector<Descriptor>& descriptors);
  int save_descriptors(std::string filename, std::vector<Descriptor>& descriptors);

  /* ---------------------------------------------------------------------------------- */

  class Descriptor {

  public:
    Descriptor();
    ~Descriptor();
    void reset();

  public:
    std::string filename;
    uint32_t average_color[3];
  };

} /* namespace fex */
#endif
